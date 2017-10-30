/* Wine Vulkan ICD implementation
 *
 * Copyright 2017 Roderick Colenbrander
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winuser.h"

#include "wine/debug.h"
#include "wine/heap.h"
#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"
#include "vulkan_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(vulkan);

/* For now default to 4 as it felt like a reasonable version feature wise to support.
 * Don't support the optional vk_icdGetPhysicalDeviceProcAddr introduced in this version
 * as it is unlikely we will implement physical device extensions, which the loader is not
 * aware off. Version 5 adds more extensive version checks. Something to tackle later.
 */
#define WINE_VULKAN_ICD_VERSION 4

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

struct vulkan_func
{
    const char *name;
    void *func;
};

static void *wine_vk_get_global_proc_addr(const char *name);

static const struct vulkan_funcs *vk_funcs = NULL;

static BOOL wine_vk_init(HINSTANCE hinst)
{
    HDC hdc = GetDC(0);

    vk_funcs =  __wine_get_vulkan_driver(hdc, WINE_VULKAN_DRIVER_VERSION);
    if (!vk_funcs)
    {
        ERR("Failed to load Wine graphics driver supporting Vulkan.\n");
        ReleaseDC(0, hdc);
        return FALSE;
    }

    DisableThreadLibraryCalls(hinst);

    ReleaseDC(0, hdc);
    return TRUE;
}

/* Helper function used for freeing an instance structure. This function supports full
 * and partial object cleanups and can thus be used for vkCreateInstance failures.
 */
static void wine_vk_instance_free(struct VkInstance_T *instance)
{
    if (!instance)
        return;

    if (instance->instance)
        vk_funcs->p_vkDestroyInstance(instance->instance, NULL /* pAllocator */);

    heap_free(instance);
}

static VkResult WINAPI wine_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance)
{
    struct VkInstance_T *instance = NULL;
    VkResult res;

    TRACE("pCreateInfo %p, pAllocator %p, pInstance %p\n", pCreateInfo, pAllocator, pInstance);

    if (pAllocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    instance = heap_alloc(sizeof(*instance));
    if (!instance)
    {
        ERR("Failed to allocate memory for instance\n");
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto err;
    }
    instance->base.loader_magic = VULKAN_ICD_MAGIC_VALUE;

    res = vk_funcs->p_vkCreateInstance(pCreateInfo, NULL /* pAllocator */, &instance->instance);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to create instance, res=%d\n", res);
        goto err;
    }

    *pInstance = instance;
    TRACE("Done, instance=%p native_instance=%p\n", instance, instance->instance);
    return VK_SUCCESS;

err:
    if (instance)
        wine_vk_instance_free(instance);

    return res;
}

static VkResult WINAPI wine_vkEnumerateInstanceExtensionProperties(const char *layer_name, uint32_t *count,
        VkExtensionProperties *properties)
{
    TRACE("%p %p %p\n", layer_name, count, properties);
    return vk_funcs->p_vkEnumerateInstanceExtensionProperties(layer_name, count, properties);
}

static PFN_vkVoidFunction WINAPI wine_vkGetInstanceProcAddr(VkInstance instance, const char *name)
{
    void *func;
    TRACE("%p %s\n", instance, debugstr_a(name));

    if (!name)
        return NULL;

    /* vkGetInstanceProcAddr can load most Vulkan functions when an instance is passed in, however
     * for a NULL instance it can only load global functions.
     */
    func = wine_vk_get_global_proc_addr(name);
    if (func)
    {
        return func;
    }
    else if (!instance)
    {
        FIXME("Global function '%s' not found\n", name);
        return NULL;
    }

    FIXME("Unsupported device or instance function: '%s'\n", name);
    return NULL;
}

void * WINAPI wine_vk_icdGetInstanceProcAddr(VkInstance instance, const char *name)
{
    TRACE("%p %s\n", instance, debugstr_a(name));

    /* Initial version of the Vulkan ICD spec required vkGetInstanceProcAddr to be
     * exported. vk_icdGetInstanceProcAddr was added later to separete ICD calls from
     * Vulkan API. One of them in our case should forward to the other, so just forward
     * to the older vkGetInstanceProcAddr.
     */
    return wine_vkGetInstanceProcAddr(instance, name);
}

VkResult WINAPI wine_vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t *supported_version)
{
    uint32_t req_version;
    TRACE("%p\n", supported_version);

    /* The spec is not clear how to handle this. Mesa drivers don't check, but it
     * is probably best to not explode. VK_INCOMPLETE seems to be the closest value.
     */
    if (!supported_version)
        return VK_INCOMPLETE;

    req_version = *supported_version;
    *supported_version = min(req_version, WINE_VULKAN_ICD_VERSION);
    TRACE("Loader requested ICD version %u, returning %u\n", req_version, *supported_version);

    return VK_SUCCESS;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    switch(reason)
    {
        case DLL_PROCESS_ATTACH:
            return wine_vk_init(hinst);

        case DLL_THREAD_ATTACH:
            break;
    }
    return TRUE;
}

static const struct vulkan_func vk_global_dispatch_table[] =
{
    {"vkCreateInstance", &wine_vkCreateInstance},
    {"vkEnumerateInstanceExtensionProperties", &wine_vkEnumerateInstanceExtensionProperties},
    {"vkGetInstanceProcAddr", &wine_vkGetInstanceProcAddr},
};

static void *wine_vk_get_global_proc_addr(const char *name)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(vk_global_dispatch_table); i++)
    {
        if (strcmp(name, vk_global_dispatch_table[i].name) == 0)
        {
            TRACE("Found name=%s in global table\n", debugstr_a(name));
            return vk_global_dispatch_table[i].func;
        }
    }
    return NULL;
}
