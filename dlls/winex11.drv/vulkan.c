/* X11DRV Vulkan implementation
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

#include "config.h"
#include "wine/port.h"

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"

#include "wine/debug.h"
#include "wine/heap.h"
#include "wine/library.h"

/* We only want host compatible structures and don't need alignment. */
#define WINE_VK_ALIGN(x)

#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"

#ifdef SONAME_LIBVULKAN

WINE_DEFAULT_DEBUG_CHANNEL(vulkan);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* All Vulkan structures use this structure for the first elements. */
struct wine_vk_structure_header
{
    VkStructureType sType;
    const void *pNext;
};

static VkResult (*pvkCreateInstance)(const VkInstanceCreateInfo *, const VkAllocationCallbacks *, VkInstance *);
static void (*pvkDestroyInstance)(VkInstance, const VkAllocationCallbacks *);
static void * (*pvkGetDeviceProcAddr)(VkDevice, const char *);
static void * (*pvkGetInstanceProcAddr)(VkInstance, const char *);

struct VkExtensionProperties winex11_vk_instance_extensions[] = {
    { "VK_KHR_surface", 1 },
    { "VK_KHR_win32_surface", 1},
};

static BOOL wine_vk_init(void)
{
    static BOOL init_done = FALSE;
    static void *vulkan_handle;

    if (init_done) return (vulkan_handle != NULL);
    init_done = TRUE;

    if (!(vulkan_handle = wine_dlopen(SONAME_LIBVULKAN, RTLD_NOW, NULL, 0))) return FALSE;

#define LOAD_FUNCPTR(f) if((p##f = wine_dlsym(vulkan_handle, #f, NULL, 0)) == NULL) return FALSE;
LOAD_FUNCPTR(vkCreateInstance)
LOAD_FUNCPTR(vkDestroyInstance)
LOAD_FUNCPTR(vkGetDeviceProcAddr)
LOAD_FUNCPTR(vkGetInstanceProcAddr)
#undef LOAD_FUNCPTR

    return TRUE;
}

/* Helper function for converting between win32 and X11 compatible VkInstanceCreateInfo.
 * Caller is responsible for allocation and cleanup of 'dst'.
 */
static VkResult wine_vk_instance_convert_create_info(const VkInstanceCreateInfo *src, VkInstanceCreateInfo *dst)
{
    int i;
    const char **enabled_extensions = NULL;

    dst->sType = src->sType;
    dst->flags = src->flags;
    dst->pApplicationInfo = src->pApplicationInfo;

    /* Application + loader can pass in a chain of extensions through pNext e.g. VK_EXT_debug_report
     * and layers (not sure why loader doesn't filter out loaders to ICD). We need to see how to handle
     * these as we can't just blindly pass structures through as some like VK_EXT_debug_report have
     * callbacks. Mesa ANV / Radv are ignoring pNext at the moment, unclear what binary blobs do.
     * Since in our case we are going through the Linux vulkan loader, the loader itself will add
     * some duplicate layers, so for now it is probably best to ignore extra extensions.
     */
    if (src->pNext)
    {
        const struct wine_vk_structure_header *header;

        for (header = src->pNext; header; header = header->pNext)
        {
            FIXME("Application requested a linked structure of type %d\n", header->sType);
        }
    }
    /* For now don't support anything. */
    dst->pNext = NULL;

    /* ICDs don't support any layers (at least at time of writing). The loader seems to not
     * filter out layer information when it reaches us. To avoid confusion by the native loader
     * we should filter.
     */
    dst->enabledLayerCount = 0;
    dst->ppEnabledLayerNames = NULL;

    if (src->enabledExtensionCount > 0)
    {
        enabled_extensions = heap_alloc(src->enabledExtensionCount * sizeof(*src->ppEnabledExtensionNames));
        if (!enabled_extensions)
        {
            ERR("Failed to allocate memory for enabled extensions\n");
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        for (i = 0; i < src->enabledExtensionCount; i++)
        {
            /* Substitute extension with X11 else copy. Long-term when we support more
             * extenions we should store these translations in a list.
             */
            if (!strcmp(src->ppEnabledExtensionNames[i], "VK_KHR_win32_surface"))
            {
                enabled_extensions[i] = "VK_KHR_xlib_surface";
            }
            else
            {
                enabled_extensions[i] = src->ppEnabledExtensionNames[i];
            }
        }
        dst->ppEnabledExtensionNames = (const char**)enabled_extensions;
    }
    dst->enabledExtensionCount = src->enabledExtensionCount;

    return VK_SUCCESS;
}

static VkResult X11DRV_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
        VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
    FIXME("stub: %p, 0x%s, 0x%s, 0x%s, 0x%s, %p\n", device, wine_dbgstr_longlong(swapchain), wine_dbgstr_longlong(timeout),
            wine_dbgstr_longlong(semaphore), wine_dbgstr_longlong(fence), pImageIndex);

    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkResult X11DRV_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
        VkInstance *pInstance)
{
    VkInstanceCreateInfo create_info;
    VkResult res;

    TRACE("pCreateInfo %p, pAllocater %p, pInstance %p\n", pCreateInfo, pAllocator, pInstance);

    if (pAllocator)
    {
        FIXME("Support for allocation callbacks not implemented yet\n");
    }

    res = wine_vk_instance_convert_create_info(pCreateInfo, &create_info);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to convert instance create info, res=%d\n", res);
        return res;
    }

    res = pvkCreateInstance(&create_info, NULL /* pAllocator */, pInstance);

    if (create_info.ppEnabledExtensionNames)
        heap_free((void*)create_info.ppEnabledExtensionNames);

    return res;
}

static VkResult X11DRV_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
        const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain)
{
    FIXME("stub: %p %p %p %p\n", device, pCreateInfo, pAllocator, pSwapchain);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkResult X11DRV_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
        const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface)
{
    FIXME("stub: %p %p %p %p\n", instance, pCreateInfo, pAllocator, pSurface);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static void X11DRV_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator)
{
    TRACE("%p %p\n", instance, pAllocator);

    if (pAllocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    pvkDestroyInstance(instance, NULL /* pAllocator */);
}

static void X11DRV_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator)
{
    FIXME("stub: %p 0x%s %p\n", instance, wine_dbgstr_longlong(surface), pAllocator);
}

static void X11DRV_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator)
{
    FIXME("stub: %p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), pAllocator);
}

static VkResult X11DRV_vkEnumerateInstanceExtensionProperties(const char *layer_name, uint32_t *count,
        VkExtensionProperties* properties)
{
    VkResult res;
    int i, num_copies;

    TRACE("layer_name %p, count %p, properties %p\n", debugstr_a(layer_name), count, properties);

    /* This shouldn't get called with pLayerName set, the ICD loader prevents it. */
    if (layer_name)
    {
        ERR("Layer enumeration not supported from ICD.\n");
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    if (!properties)
    {
        /* For now we only report surface extensions, long-term this needs to be
         * an intersection between what the native library supports and what thunks
         * we have.
         */
        *count = ARRAY_SIZE(winex11_vk_instance_extensions);
        return VK_SUCCESS;
    }

    if (*count < ARRAY_SIZE(winex11_vk_instance_extensions))
    {
        /* Incomplete is a type of success used to signal the application
         * that not all devices got copied.
         */
        num_copies = *count;
        res = VK_INCOMPLETE;
    }
    else
    {
        num_copies = ARRAY_SIZE(winex11_vk_instance_extensions);
        res = VK_SUCCESS;
    }

    for (i = 0; i < num_copies; i++)
    {
        memcpy(&properties[i], &winex11_vk_instance_extensions[i], sizeof(winex11_vk_instance_extensions[i]));
    }

    TRACE("Result %d, extensions copied %d\n", res, num_copies);
    return res;
}

static void * X11DRV_vkGetDeviceProcAddr(VkDevice device, const char *name)
{
    TRACE("%p, %s\n", device, debugstr_a(name));
    return pvkGetDeviceProcAddr(device, name);
}

static void * X11DRV_vkGetInstanceProcAddr(VkInstance instance, const char *name)
{
    TRACE("%p, %s\n", instance, debugstr_a(name));
    return pvkGetInstanceProcAddr(instance, name);
}

static VkResult X11DRV_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        VkSurfaceCapabilitiesKHR *pSurfaceCapabilities)
{
    FIXME("stub: %p, 0x%s, %p\n", physicalDevice, wine_dbgstr_longlong(surface), pSurfaceCapabilities);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkResult X11DRV_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats)
{
    FIXME("stub: %p, 0x%s, %p, %p\n", physicalDevice, wine_dbgstr_longlong(surface), pSurfaceFormatCount, pSurfaceFormats);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkResult X11DRV_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes)
{
    FIXME("stub: %p, 0x%s, %p, %p\n", physicalDevice, wine_dbgstr_longlong(surface), pPresentModeCount, pPresentModes);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkResult X11DRV_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
        VkSurfaceKHR surface, VkBool32 *pSupported)
{
    FIXME("stub: %p, %u, 0x%s, %p\n", physicalDevice, queueFamilyIndex, wine_dbgstr_longlong(surface), pSupported);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkBool32 X11DRV_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
        uint32_t queueFamilyIndex)
{
    FIXME("stub %p %u\n", physicalDevice, queueFamilyIndex);
    return VK_FALSE;
}

static VkResult X11DRV_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
        VkImage *pSwapchainImages)
{
    FIXME("stub: %p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), pSwapchainImageCount, pSwapchainImages);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

static VkResult X11DRV_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
    FIXME("stub: %p, %p\n", queue, pPresentInfo);
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}


static struct vulkan_funcs vulkan_funcs =
{
    X11DRV_vkAcquireNextImageKHR,
    X11DRV_vkCreateInstance,
    X11DRV_vkCreateSwapchainKHR,
    X11DRV_vkCreateWin32SurfaceKHR,
    X11DRV_vkDestroyInstance,
    X11DRV_vkDestroySurfaceKHR,
    X11DRV_vkDestroySwapchainKHR,
    X11DRV_vkEnumerateInstanceExtensionProperties,
    X11DRV_vkGetDeviceProcAddr,
    X11DRV_vkGetInstanceProcAddr,
    X11DRV_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    X11DRV_vkGetPhysicalDeviceSurfaceFormatsKHR,
    X11DRV_vkGetPhysicalDeviceSurfacePresentModesKHR,
    X11DRV_vkGetPhysicalDeviceSurfaceSupportKHR,
    X11DRV_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    X11DRV_vkGetSwapchainImagesKHR,
    X11DRV_vkQueuePresentKHR
};

const struct vulkan_funcs *get_vulkan_driver(UINT version)
{
    if (version != WINE_VULKAN_DRIVER_VERSION)
    {
        ERR("version mismatch, vulkan wants %u but driver has %u\n", version, WINE_VULKAN_DRIVER_VERSION);
        return NULL;
    }

    if (wine_vk_init())
        return &vulkan_funcs;

    return NULL;
}

#else /* No vulkan */

const struct vulkan_funcs *get_vulkan_driver(UINT version)
{
    return NULL;
}

#endif /* SONAME_LIBVULKAN */
