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

static void *wine_vk_get_global_proc_addr(const char *name);
static void wine_vk_physical_device_free(struct VkPhysicalDevice_T *phys_dev);

static const struct vulkan_funcs *vk_funcs = NULL;

/* Helper function used for freeing a device structure. This function supports full
 * and partial object cleanups and can thus be used vkCreateDevice failures.
 */
static void wine_vk_device_free(struct VkDevice_T *device)
{
    if (!device)
        return;

    if (device->queues)
    {
        int i;
        for (i = 0; i < device->max_queue_families; i++)
        {
            if (device->queues[i])
                heap_free(device->queues[i]);
        }
        heap_free(device->queues);
        device->queues = NULL;
    }

    if (device->queue_count)
        heap_free(device->queue_count);

    if (device->device && device->funcs.p_vkDestroyDevice)
    {
        device->funcs.p_vkDestroyDevice(device->device, NULL /* pAllocator */);
    }

    heap_free(device);
}

/* Helper function for release command buffers. */
static void wine_vk_device_free_command_buffers(VkDevice device, VkCommandPool pool, uint32_t count, const VkCommandBuffer *buffers)
{
    int i;

    /* To avoid have to wrap all command buffers just loop over them one by one. */
    for (i = 0; i < count; i++)
    {
        if (buffers[i]->command_buffer)
            device->funcs.p_vkFreeCommandBuffers(device->device, pool, 1, &buffers[i]->command_buffer);

        heap_free(buffers[i]);
    }
}

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

/* Helper function to create queues for a given family index. */
static struct VkQueue_T *wine_vk_device_alloc_queues(struct VkDevice_T *device, uint32_t fam_index, uint32_t queue_count)
{
    int i;

    struct VkQueue_T *queues = heap_alloc(sizeof(struct VkQueue_T)*queue_count);
    if (!queues)
    {
        ERR("Failed to allocate memory for queues\n");
        return NULL;
    }

    for (i = 0; i < queue_count; i++)
    {
        struct VkQueue_T *queue = &queues[i];
        queue->device = device;

        /* The native device was already allocated with the required number of queues, 
         * so just fetch them from there.
         */
        device->funcs.p_vkGetDeviceQueue(device->device, fam_index, i, &queue->queue);

        /* Set special header for ICD loader. */
        ((struct wine_vk_base*)queue)->loader_magic = VULKAN_ICD_MAGIC_VALUE;
    }

    return queues;
}

static struct VkPhysicalDevice_T *wine_vk_instance_alloc_physical_device(struct VkInstance_T *instance, VkPhysicalDevice phys_dev_host)
{
    struct VkPhysicalDevice_T *phys_dev;
    uint32_t num_host_properties, num_properties = 0;
    VkExtensionProperties *host_properties = NULL;
    VkResult res;
    int i, j;

    phys_dev = heap_alloc(sizeof(*phys_dev));
    if (!phys_dev)
        return NULL;

    phys_dev->base.loader_magic = VULKAN_ICD_MAGIC_VALUE;
    phys_dev->instance = instance;
    phys_dev->phys_dev = phys_dev_host;

    res = instance->funcs.p_vkEnumerateDeviceExtensionProperties(phys_dev_host, NULL, &num_host_properties, NULL);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to enumerate device extensions, res=%d\n", res);
        goto err;
    }

    host_properties = heap_alloc(num_host_properties * sizeof(*host_properties));
    if (!host_properties)
    {
        ERR("Failed to allocate memory for device properties!\n");
        goto err;
    }

    res = instance->funcs.p_vkEnumerateDeviceExtensionProperties(phys_dev_host, NULL, &num_host_properties, host_properties);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to enumerate device extensions, res=%d\n", res);
        goto err;
    }

    /* Count list of extensions for which we have an implementation.
     * TODO: perform translation for platform specific extensions.
     */
    for (i = 0; i < num_host_properties; i++)
    {
        if (wine_vk_device_extension_supported(host_properties[i].extensionName))
        {
            TRACE("Enabling extension '%s' for phys_dev %p\n", host_properties[i].extensionName, phys_dev);
            num_properties++;
        }
        else
            TRACE("Skipping extension '%s', no implementation found in winevulkan.\n", host_properties[i].extensionName);
    }

    TRACE("Host supported extensions %d, Wine supported extensions %d\n", num_host_properties, num_properties);

    phys_dev->properties = heap_alloc(num_properties * sizeof(*phys_dev->properties));
    if (!phys_dev->properties)
    {
        ERR("Failed to allocate memory for device properties!\n");
        goto err;
    }

    for (i = 0, j = 0; i < num_host_properties; i++)
    {
        if (wine_vk_device_extension_supported(host_properties[i].extensionName))
        {
            memcpy(&phys_dev->properties[j], &host_properties[i], sizeof(*phys_dev->properties));
            j++;
        }
    }
    phys_dev->num_properties = num_properties;

    heap_free(host_properties);
    return phys_dev;

err:
    wine_vk_physical_device_free(phys_dev);
    if (host_properties) heap_free(host_properties);

    return NULL;
}

/* Helper function which stores wrapped physical devices in the instance object. */
static VkResult wine_vk_instance_load_physical_devices(struct VkInstance_T *instance)
{
    VkResult res;
    struct VkPhysicalDevice_T **tmp_phys_devs = NULL;
    unsigned int i;
    uint32_t num_phys_devs = 0;

    res = instance->funcs.p_vkEnumeratePhysicalDevices(instance->instance, &num_phys_devs, NULL);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to enumerate physical devices, res=%d\n", res);
        return res;
    }

    /* Don't bother with any of the rest if the system just lacks devices. */
    if (num_phys_devs == 0)
    {
        instance->num_phys_devs = 0;
        instance->phys_devs_initialized = TRUE;
        return VK_SUCCESS;
    }

    tmp_phys_devs = heap_alloc(num_phys_devs * sizeof(*tmp_phys_devs));
    if (!tmp_phys_devs)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    res = instance->funcs.p_vkEnumeratePhysicalDevices(instance->instance, &num_phys_devs, tmp_phys_devs);
    if (res != VK_SUCCESS)
        goto err;

    instance->phys_devs = heap_alloc(num_phys_devs * sizeof(*instance->phys_devs));
    if (!instance->phys_devs)
    {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto err;
    }

    /* Wrap each native physical device handle into a dispatchable object for the ICD loader. */
    for (i = 0; i < num_phys_devs; i++)
    {
        struct VkPhysicalDevice_T *phys_dev = wine_vk_instance_alloc_physical_device(instance, tmp_phys_devs[i]);
        if (!phys_dev)
        {
            ERR("Unable to allocate memory for physical device!\n");
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto err;
        }

        instance->phys_devs[i] = phys_dev;
        instance->num_phys_devs = i;
    }
    instance->num_phys_devs = num_phys_devs;
    instance->phys_devs_initialized = TRUE;

    heap_free(tmp_phys_devs);
    return VK_SUCCESS;

err:
    if (tmp_phys_devs)
        heap_free(tmp_phys_devs);

    if (instance->phys_devs)
    {
        for (i = 0; i < instance->num_phys_devs; i++)
        {
            wine_vk_physical_device_free(instance->phys_devs[i]);
            instance->phys_devs[i] = NULL;
        }
        heap_free(instance->phys_devs);
        instance->num_phys_devs = 0;
        instance->phys_devs = NULL;
        instance->phys_devs_initialized = FALSE;
    }

    return res;
}

/* Helper function used for freeing an instance structure. This function supports full
 * and partial object cleanups and can thus be used for vkCreateInstance failures.
 */
static void wine_vk_instance_free(struct VkInstance_T *instance)
{
    if (!instance)
        return;

    if (instance->phys_devs)
    {
        unsigned int i;

        for (i = 0; i < instance->num_phys_devs; i++)
        {
            wine_vk_physical_device_free(instance->phys_devs[i]);
        }
        heap_free(instance->phys_devs);
    }

    if (instance->instance)
        vk_funcs->p_vkDestroyInstance(instance->instance, NULL /* pAllocator */);

    heap_free(instance);
}

static void wine_vk_physical_device_free(struct VkPhysicalDevice_T *phys_dev)
{
    if (!phys_dev)
        return;

    if (phys_dev->properties)
        heap_free(phys_dev->properties);

    heap_free(phys_dev);
}

VkResult WINAPI wine_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
        VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
{
    TRACE("%p, 0x%s, 0x%s, 0x%s, 0x%s, %p\n", device, wine_dbgstr_longlong(swapchain), wine_dbgstr_longlong(timeout),
            wine_dbgstr_longlong(semaphore), wine_dbgstr_longlong(fence), pImageIndex);

    return vk_funcs->p_vkAcquireNextImageKHR(device->device, swapchain, timeout, semaphore, fence, pImageIndex);
}

VkResult WINAPI wine_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
        VkCommandBuffer *buffers)
{
    VkResult res = VK_SUCCESS;
    int i;

    TRACE("%p %p %p\n", device, pAllocateInfo, buffers);

    /* The application provides an array of buffers, we just clear it for error handling reasons. */
    memset(buffers, 0, sizeof(*buffers)*pAllocateInfo->commandBufferCount);

    for (i = 0; i < pAllocateInfo->commandBufferCount; i++)
    {
#if defined(USE_STRUCT_CONVERSION)
        VkCommandBufferAllocateInfo_host allocate_info;
#else
        VkCommandBufferAllocateInfo allocate_info;
#endif
        /* TODO: future extensions (none yet) may require us to do conversions on pNext. */
        allocate_info.pNext = pAllocateInfo->pNext;
        allocate_info.sType = pAllocateInfo->sType;
        allocate_info.commandPool = pAllocateInfo->commandPool;
        allocate_info.level = pAllocateInfo->level;
        allocate_info.commandBufferCount = 1;

        TRACE("Creating command buffer %d, pool 0x%s, level %d\n", i, wine_dbgstr_longlong(allocate_info.commandPool),
                allocate_info.level);
        buffers[i] = heap_alloc(sizeof(struct VkCommandBuffer_T));
        if (!buffers[i])
        {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            break;
        }

        buffers[i]->base.loader_magic = VULKAN_ICD_MAGIC_VALUE;
        buffers[i]->device = device;
        res = device->funcs.p_vkAllocateCommandBuffers(device->device, &allocate_info, &buffers[i]->command_buffer);
        if (res != VK_SUCCESS)
        {
            ERR("Failed to allocate command buffer, res=%d\n", res);
            break;
        }
    }

    if (res != VK_SUCCESS)
    {
        wine_vk_device_free_command_buffers(device, pAllocateInfo->commandPool, i, buffers);
        return res;
    }

    return VK_SUCCESS;
}

void WINAPI wine_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
        const VkCommandBuffer *pCommandBuffers)
{
    VkCommandBuffer *buffers;
    int i;

    TRACE("%p %u %p\n", commandBuffer, commandBufferCount, pCommandBuffers);

    if (!pCommandBuffers || !commandBufferCount)
        return;

    /* Unfortunately we need a temporary buffer as our command buffers are wrapped.
     * This call is called often. Maybe we should use alloca? We don't need much memory
     * space and it needs to be cleaned up after the call anyway.
     */
    buffers = heap_alloc(commandBufferCount * sizeof(VkCommandBuffer));
    if (!buffers)
    {
        ERR("Failed to allocate memory for temporary command buffers\n");
        return;
    }

    for (i = 0; i < commandBufferCount; i++)
        buffers[i] = pCommandBuffers[i]->command_buffer;

    commandBuffer->device->funcs.p_vkCmdExecuteCommands(commandBuffer->command_buffer, commandBufferCount, buffers);

    heap_free(buffers);
}

VkResult WINAPI wine_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
        const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
    struct VkDevice_T *device = NULL;
    uint32_t max_queue_families;
    VkResult res;
    int i;

    TRACE("%p %p %p %p\n", physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (pAllocator)
    {
        FIXME("Support for allocation callbacks not implemented yet\n");
    }

    device = heap_alloc(sizeof(*device));
    if (!device)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    device->base.loader_magic = VULKAN_ICD_MAGIC_VALUE;

    /* At least for now we can directly pass pCreateInfo through. All extensions we report
     * should be compatible. In addition the loader is supposed to santize values e.g. layers.
     */
    res = physicalDevice->instance->funcs.p_vkCreateDevice(physicalDevice->phys_dev, pCreateInfo, NULL /* pAllocator */,
            &device->device);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to create device\n");
        goto err;
    }

    device->phys_dev = physicalDevice;

    /* Just load all function pointers we are aware off. The loader takes care of filtering.
     * We use vkGetDeviceProcAddr as opposed to vkGetInstanceProcAddr for efficiency reasons
     * as functions pass through fewer dispatch tables within the loader.
     */
#define USE_VK_FUNC(name) \
    device->funcs.p_##name = (void*)vk_funcs->p_vkGetDeviceProcAddr(device->device, #name); \
    if (device->funcs.p_##name == NULL) \
        TRACE("Not found %s\n", #name);
    ALL_VK_DEVICE_FUNCS()
#undef USE_VK_FUNC

    /* We need to cache all queues within the device as each requires wrapping since queues are
     * dispatchable objects.
     */
    physicalDevice->instance->funcs.p_vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->phys_dev,
            &max_queue_families, NULL);
    device->max_queue_families = max_queue_families;
    TRACE("Max queue families: %d\n", device->max_queue_families);

    device->queues = heap_alloc(sizeof(*device->queues)*max_queue_families);
    if (!device->queues)
    {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto err;
    }

    device->queue_count = heap_alloc(sizeof(*device->queue_count)*max_queue_families);
    if (!device->queue_count)
    {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto err;
    }

    for (i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
    {
        uint32_t fam_index = pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex;
        uint32_t queue_count = pCreateInfo->pQueueCreateInfos[i].queueCount;

        TRACE("queueFamilyIndex %d, queueCount %d\n", fam_index, queue_count);

        device->queues[fam_index] = wine_vk_device_alloc_queues(device, fam_index, queue_count);
        if (!device->queues[fam_index])
        {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            ERR("Failed to allocate memory for queues\n");
            goto err;
        }
        device->queue_count[fam_index] = queue_count;
    }


    *pDevice = device;
    return VK_SUCCESS;

err:
    wine_vk_device_free(device);
    return res;
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

    /* Load all instance functions we are aware of. Note the loader takes care
     * of any filtering for extensions which were not requested, but which the
     * ICD may support.
     */
#define USE_VK_FUNC(name) \
    instance->funcs.p_##name = (void*)vk_funcs->p_vkGetInstanceProcAddr(instance->instance, #name);
    ALL_VK_INSTANCE_FUNCS()
#undef USE_VK_FUNC

    instance->phys_devs_initialized = FALSE;

    *pInstance = instance;
    TRACE("Done, instance=%p native_instance=%p\n", instance, instance->instance);
    return VK_SUCCESS;

err:
    if (instance)
        wine_vk_instance_free(instance);

    return res;
}

#if defined(USE_STRUCT_CONVERSION)
static inline void convert_VkSwapchainCreateInfoKHR_win_to_host(const VkSwapchainCreateInfoKHR *in, VkSwapchainCreateInfoKHR_host *out)
{
    if (!in) return;

    out->sType = in->sType;
    out->pNext = in->pNext;
    out->flags = in->flags;
    out->surface = in->surface;
    out->minImageCount = in->minImageCount;
    out->imageFormat = in->imageFormat;
    out->imageColorSpace = in->imageColorSpace;
    out->imageExtent = in->imageExtent;
    out->imageArrayLayers = in->imageArrayLayers;
    out->imageUsage = in->imageUsage;
    out->imageSharingMode = in->imageSharingMode;
    out->queueFamilyIndexCount = in->queueFamilyIndexCount;
    out->pQueueFamilyIndices = in->pQueueFamilyIndices;
    out->preTransform = in->preTransform;
    out->compositeAlpha = in->compositeAlpha;
    out->presentMode = in->presentMode;
    out->clipped = in->clipped;
    out->oldSwapchain = in->oldSwapchain;
}
#endif

VkResult WINAPI wine_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
        const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain)
{
#if defined(USE_STRUCT_CONVERSION)
    VkSwapchainCreateInfoKHR_host pCreateInfo_host;
    TRACE("%p %p %p %p\n", device, pCreateInfo, pAllocator, pSwapchain);

    if (pAllocator)
        FIXME("Support allocation allocators\n");

    convert_VkSwapchainCreateInfoKHR_win_to_host(pCreateInfo, &pCreateInfo_host);

    /* Wine graphics driver layer only uses structs in host format. */
    return vk_funcs->p_vkCreateSwapchainKHR(device->device, (VkSwapchainCreateInfoKHR*)&pCreateInfo_host, pAllocator,
            pSwapchain);
#else
    TRACE("%p %p %p %p\n", device, pCreateInfo, pAllocator, pSwapchain);

    if (pAllocator)
        FIXME("Support allocation allocators\n");

    return vk_funcs->p_vkCreateSwapchainKHR(device->device, pCreateInfo, pAllocator, pSwapchain);
#endif
}

VkResult WINAPI wine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    TRACE("%p %p %p %p\n", instance, pCreateInfo, pAllocator, pSurface);

    if (pAllocator)
        FIXME("Support allocation allocators\n");

    return vk_funcs->p_vkCreateWin32SurfaceKHR(instance->instance, pCreateInfo, NULL /* pAllocator */, pSurface);
}

void WINAPI wine_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
    TRACE("%p %p\n", device, pAllocator);

    if (pAllocator)
    {
        FIXME("Support for allocation callbacks not implemented yet\n");
    }

    wine_vk_device_free(device);
}

void WINAPI wine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator)
{
    TRACE("%p, %p\n", instance, pAllocator);

    if (pAllocator)
        FIXME("Support allocation allocators\n");

    wine_vk_instance_free(instance);
}

void WINAPI wine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator)
{
    TRACE("%p, 0x%s, %p\n", instance, wine_dbgstr_longlong(surface), pAllocator);

    if (pAllocator)
        FIXME("Support allocation allocators\n");

    vk_funcs->p_vkDestroySurfaceKHR(instance->instance, surface, NULL /* pAllocator */);
}

void WINAPI wine_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator)
{
    TRACE("%p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), pAllocator);

    if (pAllocator)
        FIXME("Support allocation allocators\n");

    vk_funcs->p_vkDestroySwapchainKHR(device->device, swapchain, NULL /* pAllocator */);
}

VkResult WINAPI wine_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice phys_dev, const char *layer_name,
        uint32_t *count, VkExtensionProperties *properties)
{
    VkResult res;
    int num_copies = 0;
    unsigned int i;

    TRACE("%p, %p, %p, %p\n", phys_dev, layer_name, count, properties);

    /* This shouldn't get called with layer_name set, the ICD loader prevents it. */
    if (layer_name)
    {
        ERR("Layer enumeration not supported from ICD.\n");
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    if (!properties)
    {
        *count = phys_dev->num_properties;
        return VK_SUCCESS;
    }

    if (*count < phys_dev->num_properties)
    {
        /* Incomplete is a type of success used to signal the application
         * that not all devices got copied.
         */
        num_copies = *count;
        res = VK_INCOMPLETE;
    }
    else
    {
        num_copies = phys_dev->num_properties;
        res = VK_SUCCESS;
    }

    for (i = 0; i < num_copies; i++)
    {
        memcpy(&properties[i], &phys_dev->properties[i], sizeof(phys_dev->properties[i]));
    }

    TRACE("Result %d, extensions copied %d\n", res, num_copies);
    return res;
}

static VkResult WINAPI wine_vkEnumerateInstanceExtensionProperties(const char *layer_name, uint32_t *count,
        VkExtensionProperties *properties)
{
    VkResult res;
    uint32_t num_properties = 0, num_host_properties = 0;
    VkExtensionProperties *host_properties = NULL;
    unsigned int i, j;

    TRACE("%p %p %p\n", layer_name, count, properties);

    /* This shouldn't get called with layer_name set, the ICD loader prevents it. */
    if (layer_name)
    {
        ERR("Layer enumeration not supported from ICD.\n");
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    res = vk_funcs->p_vkEnumerateInstanceExtensionProperties(NULL, &num_host_properties, NULL);
    if (res != VK_SUCCESS)
        return res;

    host_properties = heap_alloc(num_host_properties * sizeof(*host_properties));
    if (!host_properties)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    res = vk_funcs->p_vkEnumerateInstanceExtensionProperties(NULL, &num_host_properties, host_properties);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to retrieve host properties, res=%d\n", res);
        heap_free(host_properties);
        return res;
    }

    /* The Wine graphics driver provides us with all extensions supported by the host side
     * including extension fixup (e.g. VK_KHR_xlib_surface -> VK_KHR_win32_surface). It is
     * up to us here to filter the list down to extension, which we have thunks for.
     */
    for (i = 0; i < num_host_properties; i++)
    {
        if (wine_vk_instance_extension_supported(host_properties[i].extensionName))
            num_properties++;
    }

    /* We only have to count. */
    if (!properties)
    {
        TRACE("Returning %d extensions\n", num_properties);
        *count = num_properties;
        heap_free(host_properties);
        return VK_SUCCESS;
    }

    for (i = 0, j = 0; i < num_host_properties && j < *count; i++)
    {
        if (wine_vk_instance_extension_supported(host_properties[i].extensionName))
        {
            TRACE("Enabling extension '%s'\n", host_properties[i].extensionName);
            memcpy(&properties[j], &host_properties[i], sizeof(*properties));
            j++;
        }
    }

    /* Return incomplete if the buffer is smaller than the number of supported extensions. */
    if (*count < num_properties)
        res = VK_INCOMPLETE;
    else
        res = VK_SUCCESS;

    heap_free(host_properties);
    return res;
}

VkResult WINAPI wine_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *device_count, VkPhysicalDevice *devices)
{
    VkResult res;
    unsigned int i;
    int num_copies = 0;

    TRACE("%p %p %p\n", instance, device_count, devices);

    /* Cache physical devices for vkEnumeratePhysicalDevices within the instance as each
     * vkPhysicalDevice is a dispatchable object, which means we need to wrap the native
     * physical device and present those the application. Applications call this function
     * multiple times first to get the number of devices, then to get the devices.
     * Cleanup happens as part of wine_vkDestroyInstance.
     */
    if (instance->phys_devs_initialized == FALSE)
    {
        res = wine_vk_instance_load_physical_devices(instance);
        if (res != VK_SUCCESS)
        {
            ERR("Failed to cache physical devices, res=%d\n", res);
            return res;
        }
    }

    if (!devices)
    {
        *device_count = instance->num_phys_devs;
        return VK_SUCCESS;
    }

    if (*device_count < instance->num_phys_devs)
    {
        /* Incomplete is a type of success used to signal the application
         * that not all devices got copied.
         */
        num_copies = *device_count;
        res = VK_INCOMPLETE;
    }
    else
    {
        num_copies = instance->num_phys_devs;
        res = VK_SUCCESS;
    }

    for (i = 0; i < num_copies; i++)
    {
        devices[i] = instance->phys_devs[i];
    }
    *device_count = num_copies;

    TRACE("Returning %d devices\n", *device_count);
    return res;
}

void WINAPI wine_vkFreeCommandBuffers(VkDevice device, VkCommandPool pool, uint32_t count,
        const VkCommandBuffer *buffers)
{
    TRACE("%p 0x%s %d %p\n", device, wine_dbgstr_longlong(pool), count, buffers);

    wine_vk_device_free_command_buffers(device, pool, count, buffers);
}

PFN_vkVoidFunction WINAPI wine_vkGetDeviceProcAddr(VkDevice device, const char *name)
{
    void *func;
    TRACE("%p, %s\n", device, debugstr_a(name));

    /* The spec leaves return value undefined for a NULL device, let's just return NULL. */
    if (!device || !name)
        return NULL;

    /* Per the spec, we are only supposed to return device functions as in functions
     * for which the first parameter is vkDevice or a child of vkDevice like a
     * vkCommanBuffer, vkQueue.
     * Loader takes are of filtering of extensions which are enabled or not.
     */
    func = wine_vk_get_device_proc_addr(name);
    if (func)
        return func;

    TRACE("Function %s not found\n", name);
    return NULL;
}

void WINAPI wine_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue)
{
    TRACE("%p %d %d %p\n", device, queueFamilyIndex, queueIndex, pQueue);

    *pQueue = &device->queues[queueFamilyIndex][queueIndex];
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

    func = wine_vk_get_instance_proc_addr(name);
    if (func) return func;

    /* vkGetInstanceProcAddr also loads any children of instance, so device functions as well. */
    func = wine_vk_get_device_proc_addr(name);
    if (func) return func;

    FIXME("Unsupported device or instance function: '%s'\n", name);
    return NULL;
}

VkResult WINAPI wine_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        VkSurfaceCapabilitiesKHR *pSurfaceCapabilities)
{
    TRACE("%p, 0x%s, %p\n", physicalDevice, wine_dbgstr_longlong(surface), pSurfaceCapabilities);
    return vk_funcs->p_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->phys_dev, surface, pSurfaceCapabilities);
}

VkResult WINAPI wine_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats)
{
    TRACE("%p, 0x%s, %p, %p\n", physicalDevice, wine_dbgstr_longlong(surface), pSurfaceFormatCount, pSurfaceFormats);
    return vk_funcs->p_vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->phys_dev, surface, pSurfaceFormatCount,
            pSurfaceFormats);
}

VkResult WINAPI wine_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
        uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes)
{
    TRACE("%p, 0x%s, %p, %p\n", physicalDevice, wine_dbgstr_longlong(surface), pPresentModeCount, pPresentModes);
    return vk_funcs->p_vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->phys_dev, surface, pPresentModeCount,
            pPresentModes);
}

VkResult WINAPI wine_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
        VkSurfaceKHR surface, VkBool32 *pSupported)
{
    TRACE("%p, %u, 0x%s, %p\n", physicalDevice, queueFamilyIndex, wine_dbgstr_longlong(surface), pSupported);
    return vk_funcs->p_vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->phys_dev, queueFamilyIndex, surface,
            pSupported);
}

VkBool32 WINAPI wine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice,
        uint32_t queueFamilyIndex)
{
    TRACE("%p %u\n", physicalDevice, queueFamilyIndex);
    return vk_funcs->p_vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice->phys_dev, queueFamilyIndex);
}

VkResult WINAPI wine_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
        VkImage *pSwapchainImages)
{
    TRACE("%p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), pSwapchainImageCount, pSwapchainImages);
    return vk_funcs->p_vkGetSwapchainImagesKHR(device->device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult WINAPI wine_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo)
{
    TRACE("%p, %p\n", queue, pPresentInfo);
    return vk_funcs->p_vkQueuePresentKHR(queue->queue, pPresentInfo);
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

VkResult WINAPI wine_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence)
{
    VkSubmitInfo *submits;
    VkResult res;
    VkCommandBuffer *command_buffers;
    int i, num_command_buffers;

    TRACE("%p %u %p 0x%s\n", queue, submitCount, pSubmits, wine_dbgstr_longlong(fence));

    if (submitCount == 0)
    {
        return queue->device->funcs.p_vkQueueSubmit(queue->queue, 0, NULL, fence);
    }

    submits = heap_alloc(sizeof(*submits)*submitCount);
    if (!submits)
    {
        ERR("Unable to allocate memory for submit buffers!\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    for (i = 0; i < submitCount; i++)
    {
        int j;

        memcpy(&submits[i], &pSubmits[i], sizeof(*submits));

        num_command_buffers = pSubmits[i].commandBufferCount;
        command_buffers = heap_alloc(sizeof(*submits)*num_command_buffers);
        if (!command_buffers)
        {
            ERR("Unable to allocate memory for comman buffers!\n");
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto err;
        }

        for (j = 0; j < num_command_buffers; j++)
        {
            command_buffers[j] = pSubmits[i].pCommandBuffers[j]->command_buffer;
        }
        submits[i].pCommandBuffers = command_buffers;
    }

    res = queue->device->funcs.p_vkQueueSubmit(queue->queue, submitCount, submits, fence);

err:
    if (submits)
    {
        for (i = 0; i < submitCount; i++)
        {
            if (submits[i].pCommandBuffers)
                heap_free((void*)submits[i].pCommandBuffers);
        }
        heap_free(submits);
    }

    TRACE("Returning %d\n", res);
    return res;
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
