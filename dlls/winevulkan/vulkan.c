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
#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"

WINE_DEFAULT_DEBUG_CHANNEL(vulkan);

/* For now default to 3 as it felt like a reasonable version feature wise to support.
 * Version 4 requires us to implement vk_icdGetPhysicalDeviceProcAddr, which I didn't
 * want to deal with just yet. It also added some more detailed API version check
 * requirements. Version 5 builds further on this. Something to tackle later.
 */
#define WINE_VULKAN_ICD_VERSION 3

static VkResult WINAPI wine_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance);
static VkResult WINAPI wine_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
        VkExtensionProperties* pProperties);
static PFN_vkVoidFunction WINAPI wine_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

struct vulkan_func
{
    const char *name;
    void *func;
};

const struct vulkan_func vk_global_dispatch_table[] = {
    {"vkCreateInstance", &wine_vkCreateInstance},
    {"vkEnumerateInstanceExtensionProperties", &wine_vkEnumerateInstanceExtensionProperties},
    {"vkGetInstanceProcAddr", &wine_vkGetInstanceProcAddr},
};

static struct vulkan_funcs *vk_funcs = NULL;

static void *wine_vk_get_global_proc_addr(const char *name)
{
    int i;
    for (i = 0; i < sizeof(vk_global_dispatch_table) / sizeof(vk_global_dispatch_table[0]); i++)
    {
        if (strcmp(name, vk_global_dispatch_table[i].name) == 0)
        {
            TRACE("Found pName=%s in global table\n", name);
            return vk_global_dispatch_table[i].func;
        }
    }
    return NULL;
}

static BOOL wine_vk_init(HINSTANCE hinst)
{
    HDC hdc = GetDC(0);

    vk_funcs =  __wine_get_vulkan_driver(hdc, WINE_VULKAN_DRIVER_VERSION);
    if (!vk_funcs)
    {
        ReleaseDC(0, hdc);
        return FALSE;
    }

    DisableThreadLibraryCalls(hinst);

    ReleaseDC(0, hdc);
    return TRUE;
}

static VkResult WINAPI wine_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance)
{
    FIXME("stub: %p %p %p\n", pCreateInfo, pAllocator, pInstance);
    return VK_ERROR_INCOMPATIBLE_DRIVER;
}

static VkResult WINAPI wine_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
        VkExtensionProperties *pProperties)
{
    FIXME("stub: %p %p %p\n", pLayerName, pPropertyCount, pProperties);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static PFN_vkVoidFunction WINAPI wine_vkGetInstanceProcAddr(VkInstance instance, const char *pName)
{
    void *func;
    TRACE("%p %s\n", instance, debugstr_a(pName));

    /* vkGetInstanceProcAddr can load most Vulkan functions when an instance is passed in, however
     * for a NULL instance it can only load global functions.
     */
    func = wine_vk_get_global_proc_addr(pName);
    if (func)
    {
        return func;
    }
    else if (!instance && !func)
    {
        FIXME("Global function '%s' not found\n", pName);
        return NULL;
    }

    FIXME("Unsupported device or instance function: '%s'\n", pName);
    return NULL;
}

void * WINAPI wine_vk_icdGetInstanceProcAddr(VkInstance instance, const char *pName)
{
    TRACE("%p %s\n", instance, debugstr_a(pName));

    /* Initial version of the Vulkan ICD spec required vkGetInstanceProcAddr to be
     * exported. vk_icdGetInstanceProcAddr was added later to separete ICD calls from
     * Vulkan API. One of them in our case should forward to the other, so just forward
     * to the older vkGetInstanceProcAddr.
     */
    return wine_vkGetInstanceProcAddr(instance, pName);
}

VkResult WINAPI wine_vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t *pSupportedVersion)
{
    uint32_t req_version;
    TRACE("%p\n", pSupportedVersion);

    /* The spec is not clear how to handle this. Mesa drivers don't check, but it
     * is probably best to not explode. VK_INCOMPLETE seems to be the closest value.
     */
    if (!pSupportedVersion)
        return VK_INCOMPLETE;

    req_version = *pSupportedVersion;
    *pSupportedVersion = min(req_version, WINE_VULKAN_ICD_VERSION);
    TRACE("Loader requested ICD version=%d, returning %d\n", req_version, *pSupportedVersion);

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