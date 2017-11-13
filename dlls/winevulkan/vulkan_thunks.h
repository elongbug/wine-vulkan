/* Automatically generated from Vulkan vk.xml; DO NOT EDIT! */

#ifndef __WINE_VULKAN_THUNKS_H
#define __WINE_VULKAN_THUNKS_H

/* For use by vk_icdGetInstanceProcAddr / vkGetInstanceProcAddr */
void *wine_vk_get_device_proc_addr(const char *name) DECLSPEC_HIDDEN;
void *wine_vk_get_instance_proc_addr(const char *name) DECLSPEC_HIDDEN;

/* Functions for which we have custom implementations outside of the thunks. */
VkResult WINAPI wine_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo, VkCommandBuffer *pCommandBuffers) DECLSPEC_HIDDEN;
VkResult WINAPI wine_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) DECLSPEC_HIDDEN;
VkResult WINAPI wine_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) DECLSPEC_HIDDEN;
void WINAPI wine_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) DECLSPEC_HIDDEN;
void WINAPI wine_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) DECLSPEC_HIDDEN;
void WINAPI wine_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator) DECLSPEC_HIDDEN;
VkResult WINAPI wine_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices) DECLSPEC_HIDDEN;
void WINAPI wine_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) DECLSPEC_HIDDEN;
PFN_vkVoidFunction WINAPI wine_vkGetDeviceProcAddr(VkDevice device, const char *pName) DECLSPEC_HIDDEN;
void WINAPI wine_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) DECLSPEC_HIDDEN;
VkBool32 WINAPI wine_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) DECLSPEC_HIDDEN;

/* For use by vkDevice and children */
struct vulkan_device_funcs
{
    VkResult (*p_vkAcquireNextImageKHR)(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t *);
    VkResult (*p_vkAllocateCommandBuffers)(VkDevice, const VkCommandBufferAllocateInfo *, VkCommandBuffer *);
    VkResult (*p_vkAllocateDescriptorSets)(VkDevice, const VkDescriptorSetAllocateInfo *, VkDescriptorSet *);
    VkResult (*p_vkAllocateMemory)(VkDevice, const VkMemoryAllocateInfo *, const VkAllocationCallbacks *, VkDeviceMemory *);
    VkResult (*p_vkBeginCommandBuffer)(VkCommandBuffer, const VkCommandBufferBeginInfo *);
    VkResult (*p_vkBindBufferMemory)(VkDevice, VkBuffer, VkDeviceMemory, VkDeviceSize);
    VkResult (*p_vkBindImageMemory)(VkDevice, VkImage, VkDeviceMemory, VkDeviceSize);
    void (*p_vkCmdBeginQuery)(VkCommandBuffer, VkQueryPool, uint32_t, VkQueryControlFlags);
    void (*p_vkCmdBeginRenderPass)(VkCommandBuffer, const VkRenderPassBeginInfo *, VkSubpassContents);
    void (*p_vkCmdBindDescriptorSets)(VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t, const VkDescriptorSet *, uint32_t, const uint32_t *);
    void (*p_vkCmdBindIndexBuffer)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkIndexType);
    void (*p_vkCmdBindPipeline)(VkCommandBuffer, VkPipelineBindPoint, VkPipeline);
    void (*p_vkCmdBindVertexBuffers)(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer *, const VkDeviceSize *);
    void (*p_vkCmdBlitImage)(VkCommandBuffer, VkImage, VkImageLayout, VkImage, VkImageLayout, uint32_t, const VkImageBlit *, VkFilter);
    void (*p_vkCmdClearAttachments)(VkCommandBuffer, uint32_t, const VkClearAttachment *, uint32_t, const VkClearRect *);
    void (*p_vkCmdClearColorImage)(VkCommandBuffer, VkImage, VkImageLayout, const VkClearColorValue *, uint32_t, const VkImageSubresourceRange *);
    void (*p_vkCmdClearDepthStencilImage)(VkCommandBuffer, VkImage, VkImageLayout, const VkClearDepthStencilValue *, uint32_t, const VkImageSubresourceRange *);
    void (*p_vkCmdCopyBuffer)(VkCommandBuffer, VkBuffer, VkBuffer, uint32_t, const VkBufferCopy *);
    void (*p_vkCmdCopyBufferToImage)(VkCommandBuffer, VkBuffer, VkImage, VkImageLayout, uint32_t, const VkBufferImageCopy *);
    void (*p_vkCmdCopyImage)(VkCommandBuffer, VkImage, VkImageLayout, VkImage, VkImageLayout, uint32_t, const VkImageCopy *);
    void (*p_vkCmdCopyImageToBuffer)(VkCommandBuffer, VkImage, VkImageLayout, VkBuffer, uint32_t, const VkBufferImageCopy *);
    void (*p_vkCmdCopyQueryPoolResults)(VkCommandBuffer, VkQueryPool, uint32_t, uint32_t, VkBuffer, VkDeviceSize, VkDeviceSize, VkQueryResultFlags);
    void (*p_vkCmdDispatch)(VkCommandBuffer, uint32_t, uint32_t, uint32_t);
    void (*p_vkCmdDispatchIndirect)(VkCommandBuffer, VkBuffer, VkDeviceSize);
    void (*p_vkCmdDraw)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t);
    void (*p_vkCmdDrawIndexed)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, int32_t, uint32_t);
    void (*p_vkCmdDrawIndexedIndirect)(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
    void (*p_vkCmdDrawIndirect)(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
    void (*p_vkCmdEndQuery)(VkCommandBuffer, VkQueryPool, uint32_t);
    void (*p_vkCmdEndRenderPass)(VkCommandBuffer);
    void (*p_vkCmdExecuteCommands)(VkCommandBuffer, uint32_t, const VkCommandBuffer *);
    void (*p_vkCmdFillBuffer)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkDeviceSize, uint32_t);
    void (*p_vkCmdNextSubpass)(VkCommandBuffer, VkSubpassContents);
    void (*p_vkCmdPipelineBarrier)(VkCommandBuffer, VkPipelineStageFlags, VkPipelineStageFlags, VkDependencyFlags, uint32_t, const VkMemoryBarrier *, uint32_t, const VkBufferMemoryBarrier *, uint32_t, const VkImageMemoryBarrier *);
    void (*p_vkCmdPushConstants)(VkCommandBuffer, VkPipelineLayout, VkShaderStageFlags, uint32_t, uint32_t, const void *);
    void (*p_vkCmdResetEvent)(VkCommandBuffer, VkEvent, VkPipelineStageFlags);
    void (*p_vkCmdResetQueryPool)(VkCommandBuffer, VkQueryPool, uint32_t, uint32_t);
    void (*p_vkCmdResolveImage)(VkCommandBuffer, VkImage, VkImageLayout, VkImage, VkImageLayout, uint32_t, const VkImageResolve *);
    void (*p_vkCmdSetBlendConstants)(VkCommandBuffer, const float[4]);
    void (*p_vkCmdSetDepthBias)(VkCommandBuffer, float, float, float);
    void (*p_vkCmdSetDepthBounds)(VkCommandBuffer, float, float);
    void (*p_vkCmdSetEvent)(VkCommandBuffer, VkEvent, VkPipelineStageFlags);
    void (*p_vkCmdSetLineWidth)(VkCommandBuffer, float);
    void (*p_vkCmdSetScissor)(VkCommandBuffer, uint32_t, uint32_t, const VkRect2D *);
    void (*p_vkCmdSetStencilCompareMask)(VkCommandBuffer, VkStencilFaceFlags, uint32_t);
    void (*p_vkCmdSetStencilReference)(VkCommandBuffer, VkStencilFaceFlags, uint32_t);
    void (*p_vkCmdSetStencilWriteMask)(VkCommandBuffer, VkStencilFaceFlags, uint32_t);
    void (*p_vkCmdSetViewport)(VkCommandBuffer, uint32_t, uint32_t, const VkViewport *);
    void (*p_vkCmdUpdateBuffer)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkDeviceSize, const void *);
    void (*p_vkCmdWaitEvents)(VkCommandBuffer, uint32_t, const VkEvent *, VkPipelineStageFlags, VkPipelineStageFlags, uint32_t, const VkMemoryBarrier *, uint32_t, const VkBufferMemoryBarrier *, uint32_t, const VkImageMemoryBarrier *);
    void (*p_vkCmdWriteTimestamp)(VkCommandBuffer, VkPipelineStageFlagBits, VkQueryPool, uint32_t);
    VkResult (*p_vkCreateBuffer)(VkDevice, const VkBufferCreateInfo *, const VkAllocationCallbacks *, VkBuffer *);
    VkResult (*p_vkCreateBufferView)(VkDevice, const VkBufferViewCreateInfo *, const VkAllocationCallbacks *, VkBufferView *);
    VkResult (*p_vkCreateCommandPool)(VkDevice, const VkCommandPoolCreateInfo *, const VkAllocationCallbacks *, VkCommandPool *);
    VkResult (*p_vkCreateComputePipelines)(VkDevice, VkPipelineCache, uint32_t, const VkComputePipelineCreateInfo *, const VkAllocationCallbacks *, VkPipeline *);
    VkResult (*p_vkCreateDescriptorPool)(VkDevice, const VkDescriptorPoolCreateInfo *, const VkAllocationCallbacks *, VkDescriptorPool *);
    VkResult (*p_vkCreateDescriptorSetLayout)(VkDevice, const VkDescriptorSetLayoutCreateInfo *, const VkAllocationCallbacks *, VkDescriptorSetLayout *);
    VkResult (*p_vkCreateEvent)(VkDevice, const VkEventCreateInfo *, const VkAllocationCallbacks *, VkEvent *);
    VkResult (*p_vkCreateFence)(VkDevice, const VkFenceCreateInfo *, const VkAllocationCallbacks *, VkFence *);
    VkResult (*p_vkCreateFramebuffer)(VkDevice, const VkFramebufferCreateInfo *, const VkAllocationCallbacks *, VkFramebuffer *);
    VkResult (*p_vkCreateGraphicsPipelines)(VkDevice, VkPipelineCache, uint32_t, const VkGraphicsPipelineCreateInfo *, const VkAllocationCallbacks *, VkPipeline *);
    VkResult (*p_vkCreateImage)(VkDevice, const VkImageCreateInfo *, const VkAllocationCallbacks *, VkImage *);
    VkResult (*p_vkCreateImageView)(VkDevice, const VkImageViewCreateInfo *, const VkAllocationCallbacks *, VkImageView *);
    VkResult (*p_vkCreatePipelineCache)(VkDevice, const VkPipelineCacheCreateInfo *, const VkAllocationCallbacks *, VkPipelineCache *);
    VkResult (*p_vkCreatePipelineLayout)(VkDevice, const VkPipelineLayoutCreateInfo *, const VkAllocationCallbacks *, VkPipelineLayout *);
    VkResult (*p_vkCreateQueryPool)(VkDevice, const VkQueryPoolCreateInfo *, const VkAllocationCallbacks *, VkQueryPool *);
    VkResult (*p_vkCreateRenderPass)(VkDevice, const VkRenderPassCreateInfo *, const VkAllocationCallbacks *, VkRenderPass *);
    VkResult (*p_vkCreateSampler)(VkDevice, const VkSamplerCreateInfo *, const VkAllocationCallbacks *, VkSampler *);
    VkResult (*p_vkCreateSemaphore)(VkDevice, const VkSemaphoreCreateInfo *, const VkAllocationCallbacks *, VkSemaphore *);
    VkResult (*p_vkCreateShaderModule)(VkDevice, const VkShaderModuleCreateInfo *, const VkAllocationCallbacks *, VkShaderModule *);
    VkResult (*p_vkCreateSwapchainKHR)(VkDevice, const VkSwapchainCreateInfoKHR *, const VkAllocationCallbacks *, VkSwapchainKHR *);
    void (*p_vkDestroyBuffer)(VkDevice, VkBuffer, const VkAllocationCallbacks *);
    void (*p_vkDestroyBufferView)(VkDevice, VkBufferView, const VkAllocationCallbacks *);
    void (*p_vkDestroyCommandPool)(VkDevice, VkCommandPool, const VkAllocationCallbacks *);
    void (*p_vkDestroyDescriptorPool)(VkDevice, VkDescriptorPool, const VkAllocationCallbacks *);
    void (*p_vkDestroyDescriptorSetLayout)(VkDevice, VkDescriptorSetLayout, const VkAllocationCallbacks *);
    void (*p_vkDestroyDevice)(VkDevice, const VkAllocationCallbacks *);
    void (*p_vkDestroyEvent)(VkDevice, VkEvent, const VkAllocationCallbacks *);
    void (*p_vkDestroyFence)(VkDevice, VkFence, const VkAllocationCallbacks *);
    void (*p_vkDestroyFramebuffer)(VkDevice, VkFramebuffer, const VkAllocationCallbacks *);
    void (*p_vkDestroyImage)(VkDevice, VkImage, const VkAllocationCallbacks *);
    void (*p_vkDestroyImageView)(VkDevice, VkImageView, const VkAllocationCallbacks *);
    void (*p_vkDestroyPipeline)(VkDevice, VkPipeline, const VkAllocationCallbacks *);
    void (*p_vkDestroyPipelineCache)(VkDevice, VkPipelineCache, const VkAllocationCallbacks *);
    void (*p_vkDestroyPipelineLayout)(VkDevice, VkPipelineLayout, const VkAllocationCallbacks *);
    void (*p_vkDestroyQueryPool)(VkDevice, VkQueryPool, const VkAllocationCallbacks *);
    void (*p_vkDestroyRenderPass)(VkDevice, VkRenderPass, const VkAllocationCallbacks *);
    void (*p_vkDestroySampler)(VkDevice, VkSampler, const VkAllocationCallbacks *);
    void (*p_vkDestroySemaphore)(VkDevice, VkSemaphore, const VkAllocationCallbacks *);
    void (*p_vkDestroyShaderModule)(VkDevice, VkShaderModule, const VkAllocationCallbacks *);
    void (*p_vkDestroySwapchainKHR)(VkDevice, VkSwapchainKHR, const VkAllocationCallbacks *);
    VkResult (*p_vkDeviceWaitIdle)(VkDevice);
    VkResult (*p_vkEndCommandBuffer)(VkCommandBuffer);
    VkResult (*p_vkFlushMappedMemoryRanges)(VkDevice, uint32_t, const VkMappedMemoryRange *);
    void (*p_vkFreeCommandBuffers)(VkDevice, VkCommandPool, uint32_t, const VkCommandBuffer *);
    VkResult (*p_vkFreeDescriptorSets)(VkDevice, VkDescriptorPool, uint32_t, const VkDescriptorSet *);
    void (*p_vkFreeMemory)(VkDevice, VkDeviceMemory, const VkAllocationCallbacks *);
    void (*p_vkGetBufferMemoryRequirements)(VkDevice, VkBuffer, VkMemoryRequirements *);
    void (*p_vkGetDeviceMemoryCommitment)(VkDevice, VkDeviceMemory, VkDeviceSize *);
    PFN_vkVoidFunction (*p_vkGetDeviceProcAddr)(VkDevice, const char *);
    void (*p_vkGetDeviceQueue)(VkDevice, uint32_t, uint32_t, VkQueue *);
    VkResult (*p_vkGetEventStatus)(VkDevice, VkEvent);
    VkResult (*p_vkGetFenceStatus)(VkDevice, VkFence);
    void (*p_vkGetImageMemoryRequirements)(VkDevice, VkImage, VkMemoryRequirements *);
    void (*p_vkGetImageSparseMemoryRequirements)(VkDevice, VkImage, uint32_t *, VkSparseImageMemoryRequirements *);
    void (*p_vkGetImageSubresourceLayout)(VkDevice, VkImage, const VkImageSubresource *, VkSubresourceLayout *);
    VkResult (*p_vkGetPipelineCacheData)(VkDevice, VkPipelineCache, size_t *, void *);
    VkResult (*p_vkGetQueryPoolResults)(VkDevice, VkQueryPool, uint32_t, uint32_t, size_t, void *, VkDeviceSize, VkQueryResultFlags);
    void (*p_vkGetRenderAreaGranularity)(VkDevice, VkRenderPass, VkExtent2D *);
    VkResult (*p_vkGetSwapchainImagesKHR)(VkDevice, VkSwapchainKHR, uint32_t *, VkImage *);
    VkResult (*p_vkInvalidateMappedMemoryRanges)(VkDevice, uint32_t, const VkMappedMemoryRange *);
    VkResult (*p_vkMapMemory)(VkDevice, VkDeviceMemory, VkDeviceSize, VkDeviceSize, VkMemoryMapFlags, void **);
    VkResult (*p_vkMergePipelineCaches)(VkDevice, VkPipelineCache, uint32_t, const VkPipelineCache *);
    VkResult (*p_vkQueueBindSparse)(VkQueue, uint32_t, const VkBindSparseInfo *, VkFence);
    VkResult (*p_vkQueuePresentKHR)(VkQueue, const VkPresentInfoKHR *);
    VkResult (*p_vkQueueSubmit)(VkQueue, uint32_t, const VkSubmitInfo *, VkFence);
    VkResult (*p_vkQueueWaitIdle)(VkQueue);
    VkResult (*p_vkResetCommandBuffer)(VkCommandBuffer, VkCommandBufferResetFlags);
    VkResult (*p_vkResetCommandPool)(VkDevice, VkCommandPool, VkCommandPoolResetFlags);
    VkResult (*p_vkResetDescriptorPool)(VkDevice, VkDescriptorPool, VkDescriptorPoolResetFlags);
    VkResult (*p_vkResetEvent)(VkDevice, VkEvent);
    VkResult (*p_vkResetFences)(VkDevice, uint32_t, const VkFence *);
    VkResult (*p_vkSetEvent)(VkDevice, VkEvent);
    void (*p_vkUnmapMemory)(VkDevice, VkDeviceMemory);
    void (*p_vkUpdateDescriptorSets)(VkDevice, uint32_t, const VkWriteDescriptorSet *, uint32_t, const VkCopyDescriptorSet *);
    VkResult (*p_vkWaitForFences)(VkDevice, uint32_t, const VkFence *, VkBool32, uint64_t);
};

/* For use by vkInstance and children */
struct vulkan_instance_funcs
{
    VkResult (*p_vkCreateDevice)(VkPhysicalDevice, const VkDeviceCreateInfo *, const VkAllocationCallbacks *, VkDevice *);
    VkResult (*p_vkCreateWin32SurfaceKHR)(VkInstance, const VkWin32SurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);
    void (*p_vkDestroyInstance)(VkInstance, const VkAllocationCallbacks *);
    void (*p_vkDestroySurfaceKHR)(VkInstance, VkSurfaceKHR, const VkAllocationCallbacks *);
    VkResult (*p_vkEnumerateDeviceExtensionProperties)(VkPhysicalDevice, const char *, uint32_t *, VkExtensionProperties *);
    VkResult (*p_vkEnumerateDeviceLayerProperties)(VkPhysicalDevice, uint32_t *, VkLayerProperties *);
    VkResult (*p_vkEnumeratePhysicalDevices)(VkInstance, uint32_t *, VkPhysicalDevice *);
    void (*p_vkGetPhysicalDeviceFeatures)(VkPhysicalDevice, VkPhysicalDeviceFeatures *);
    void (*p_vkGetPhysicalDeviceFormatProperties)(VkPhysicalDevice, VkFormat, VkFormatProperties *);
    VkResult (*p_vkGetPhysicalDeviceImageFormatProperties)(VkPhysicalDevice, VkFormat, VkImageType, VkImageTiling, VkImageUsageFlags, VkImageCreateFlags, VkImageFormatProperties *);
    void (*p_vkGetPhysicalDeviceMemoryProperties)(VkPhysicalDevice, VkPhysicalDeviceMemoryProperties *);
    void (*p_vkGetPhysicalDeviceProperties)(VkPhysicalDevice, VkPhysicalDeviceProperties *);
    void (*p_vkGetPhysicalDeviceQueueFamilyProperties)(VkPhysicalDevice, uint32_t *, VkQueueFamilyProperties *);
    void (*p_vkGetPhysicalDeviceSparseImageFormatProperties)(VkPhysicalDevice, VkFormat, VkImageType, VkSampleCountFlagBits, VkImageUsageFlags, VkImageTiling, uint32_t *, VkSparseImageFormatProperties *);
    VkResult (*p_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(VkPhysicalDevice, VkSurfaceKHR, VkSurfaceCapabilitiesKHR *);
    VkResult (*p_vkGetPhysicalDeviceSurfaceFormatsKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t *, VkSurfaceFormatKHR *);
    VkResult (*p_vkGetPhysicalDeviceSurfacePresentModesKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t *, VkPresentModeKHR *);
    VkResult (*p_vkGetPhysicalDeviceSurfaceSupportKHR)(VkPhysicalDevice, uint32_t, VkSurfaceKHR, VkBool32 *);
    VkBool32 (*p_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice, uint32_t);
};

#define ALL_VK_DEVICE_FUNCS() \
    USE_VK_FUNC(vkAcquireNextImageKHR) \
    USE_VK_FUNC(vkAllocateCommandBuffers) \
    USE_VK_FUNC(vkAllocateDescriptorSets) \
    USE_VK_FUNC(vkAllocateMemory) \
    USE_VK_FUNC(vkBeginCommandBuffer) \
    USE_VK_FUNC(vkBindBufferMemory) \
    USE_VK_FUNC(vkBindImageMemory) \
    USE_VK_FUNC(vkCmdBeginQuery) \
    USE_VK_FUNC(vkCmdBeginRenderPass) \
    USE_VK_FUNC(vkCmdBindDescriptorSets) \
    USE_VK_FUNC(vkCmdBindIndexBuffer) \
    USE_VK_FUNC(vkCmdBindPipeline) \
    USE_VK_FUNC(vkCmdBindVertexBuffers) \
    USE_VK_FUNC(vkCmdBlitImage) \
    USE_VK_FUNC(vkCmdClearAttachments) \
    USE_VK_FUNC(vkCmdClearColorImage) \
    USE_VK_FUNC(vkCmdClearDepthStencilImage) \
    USE_VK_FUNC(vkCmdCopyBuffer) \
    USE_VK_FUNC(vkCmdCopyBufferToImage) \
    USE_VK_FUNC(vkCmdCopyImage) \
    USE_VK_FUNC(vkCmdCopyImageToBuffer) \
    USE_VK_FUNC(vkCmdCopyQueryPoolResults) \
    USE_VK_FUNC(vkCmdDispatch) \
    USE_VK_FUNC(vkCmdDispatchIndirect) \
    USE_VK_FUNC(vkCmdDraw) \
    USE_VK_FUNC(vkCmdDrawIndexed) \
    USE_VK_FUNC(vkCmdDrawIndexedIndirect) \
    USE_VK_FUNC(vkCmdDrawIndirect) \
    USE_VK_FUNC(vkCmdEndQuery) \
    USE_VK_FUNC(vkCmdEndRenderPass) \
    USE_VK_FUNC(vkCmdExecuteCommands) \
    USE_VK_FUNC(vkCmdFillBuffer) \
    USE_VK_FUNC(vkCmdNextSubpass) \
    USE_VK_FUNC(vkCmdPipelineBarrier) \
    USE_VK_FUNC(vkCmdPushConstants) \
    USE_VK_FUNC(vkCmdResetEvent) \
    USE_VK_FUNC(vkCmdResetQueryPool) \
    USE_VK_FUNC(vkCmdResolveImage) \
    USE_VK_FUNC(vkCmdSetBlendConstants) \
    USE_VK_FUNC(vkCmdSetDepthBias) \
    USE_VK_FUNC(vkCmdSetDepthBounds) \
    USE_VK_FUNC(vkCmdSetEvent) \
    USE_VK_FUNC(vkCmdSetLineWidth) \
    USE_VK_FUNC(vkCmdSetScissor) \
    USE_VK_FUNC(vkCmdSetStencilCompareMask) \
    USE_VK_FUNC(vkCmdSetStencilReference) \
    USE_VK_FUNC(vkCmdSetStencilWriteMask) \
    USE_VK_FUNC(vkCmdSetViewport) \
    USE_VK_FUNC(vkCmdUpdateBuffer) \
    USE_VK_FUNC(vkCmdWaitEvents) \
    USE_VK_FUNC(vkCmdWriteTimestamp) \
    USE_VK_FUNC(vkCreateBuffer) \
    USE_VK_FUNC(vkCreateBufferView) \
    USE_VK_FUNC(vkCreateCommandPool) \
    USE_VK_FUNC(vkCreateComputePipelines) \
    USE_VK_FUNC(vkCreateDescriptorPool) \
    USE_VK_FUNC(vkCreateDescriptorSetLayout) \
    USE_VK_FUNC(vkCreateEvent) \
    USE_VK_FUNC(vkCreateFence) \
    USE_VK_FUNC(vkCreateFramebuffer) \
    USE_VK_FUNC(vkCreateGraphicsPipelines) \
    USE_VK_FUNC(vkCreateImage) \
    USE_VK_FUNC(vkCreateImageView) \
    USE_VK_FUNC(vkCreatePipelineCache) \
    USE_VK_FUNC(vkCreatePipelineLayout) \
    USE_VK_FUNC(vkCreateQueryPool) \
    USE_VK_FUNC(vkCreateRenderPass) \
    USE_VK_FUNC(vkCreateSampler) \
    USE_VK_FUNC(vkCreateSemaphore) \
    USE_VK_FUNC(vkCreateShaderModule) \
    USE_VK_FUNC(vkCreateSwapchainKHR) \
    USE_VK_FUNC(vkDestroyBuffer) \
    USE_VK_FUNC(vkDestroyBufferView) \
    USE_VK_FUNC(vkDestroyCommandPool) \
    USE_VK_FUNC(vkDestroyDescriptorPool) \
    USE_VK_FUNC(vkDestroyDescriptorSetLayout) \
    USE_VK_FUNC(vkDestroyDevice) \
    USE_VK_FUNC(vkDestroyEvent) \
    USE_VK_FUNC(vkDestroyFence) \
    USE_VK_FUNC(vkDestroyFramebuffer) \
    USE_VK_FUNC(vkDestroyImage) \
    USE_VK_FUNC(vkDestroyImageView) \
    USE_VK_FUNC(vkDestroyPipeline) \
    USE_VK_FUNC(vkDestroyPipelineCache) \
    USE_VK_FUNC(vkDestroyPipelineLayout) \
    USE_VK_FUNC(vkDestroyQueryPool) \
    USE_VK_FUNC(vkDestroyRenderPass) \
    USE_VK_FUNC(vkDestroySampler) \
    USE_VK_FUNC(vkDestroySemaphore) \
    USE_VK_FUNC(vkDestroyShaderModule) \
    USE_VK_FUNC(vkDestroySwapchainKHR) \
    USE_VK_FUNC(vkDeviceWaitIdle) \
    USE_VK_FUNC(vkEndCommandBuffer) \
    USE_VK_FUNC(vkFlushMappedMemoryRanges) \
    USE_VK_FUNC(vkFreeCommandBuffers) \
    USE_VK_FUNC(vkFreeDescriptorSets) \
    USE_VK_FUNC(vkFreeMemory) \
    USE_VK_FUNC(vkGetBufferMemoryRequirements) \
    USE_VK_FUNC(vkGetDeviceMemoryCommitment) \
    USE_VK_FUNC(vkGetDeviceProcAddr) \
    USE_VK_FUNC(vkGetDeviceQueue) \
    USE_VK_FUNC(vkGetEventStatus) \
    USE_VK_FUNC(vkGetFenceStatus) \
    USE_VK_FUNC(vkGetImageMemoryRequirements) \
    USE_VK_FUNC(vkGetImageSparseMemoryRequirements) \
    USE_VK_FUNC(vkGetImageSubresourceLayout) \
    USE_VK_FUNC(vkGetPipelineCacheData) \
    USE_VK_FUNC(vkGetQueryPoolResults) \
    USE_VK_FUNC(vkGetRenderAreaGranularity) \
    USE_VK_FUNC(vkGetSwapchainImagesKHR) \
    USE_VK_FUNC(vkInvalidateMappedMemoryRanges) \
    USE_VK_FUNC(vkMapMemory) \
    USE_VK_FUNC(vkMergePipelineCaches) \
    USE_VK_FUNC(vkQueueBindSparse) \
    USE_VK_FUNC(vkQueuePresentKHR) \
    USE_VK_FUNC(vkQueueSubmit) \
    USE_VK_FUNC(vkQueueWaitIdle) \
    USE_VK_FUNC(vkResetCommandBuffer) \
    USE_VK_FUNC(vkResetCommandPool) \
    USE_VK_FUNC(vkResetDescriptorPool) \
    USE_VK_FUNC(vkResetEvent) \
    USE_VK_FUNC(vkResetFences) \
    USE_VK_FUNC(vkSetEvent) \
    USE_VK_FUNC(vkUnmapMemory) \
    USE_VK_FUNC(vkUpdateDescriptorSets) \
    USE_VK_FUNC(vkWaitForFences)

#define ALL_VK_INSTANCE_FUNCS() \
    USE_VK_FUNC(vkCreateDevice)\
    USE_VK_FUNC(vkCreateWin32SurfaceKHR)\
    USE_VK_FUNC(vkDestroyInstance)\
    USE_VK_FUNC(vkDestroySurfaceKHR)\
    USE_VK_FUNC(vkEnumerateDeviceExtensionProperties)\
    USE_VK_FUNC(vkEnumerateDeviceLayerProperties)\
    USE_VK_FUNC(vkEnumeratePhysicalDevices)\
    USE_VK_FUNC(vkGetPhysicalDeviceFeatures)\
    USE_VK_FUNC(vkGetPhysicalDeviceFormatProperties)\
    USE_VK_FUNC(vkGetPhysicalDeviceImageFormatProperties)\
    USE_VK_FUNC(vkGetPhysicalDeviceMemoryProperties)\
    USE_VK_FUNC(vkGetPhysicalDeviceProperties)\
    USE_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties)\
    USE_VK_FUNC(vkGetPhysicalDeviceSparseImageFormatProperties)\
    USE_VK_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)\
    USE_VK_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR)\
    USE_VK_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR)\
    USE_VK_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR)\
    USE_VK_FUNC(vkGetPhysicalDeviceWin32PresentationSupportKHR)

#endif /* __WINE_VULKAN_THUNKS_H */
