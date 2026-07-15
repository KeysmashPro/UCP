/* Vulkan.c */

#include "defines.h"
#include "callback.h"
#include "vulkan_header.h"
#include "../lib/volk/volk.h"

#include "vulkan_init.c"
#include "vulkan_compute.c"
#include "shaders/shaderdump.h"


/* SWAPCHAIN & IMAGES MANAGEMENT */

void create_swap_chain(i32 width, i32 height)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult capResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.physicalDevice, ctx.surface, &capabilities);
    if (capResult != VKS) { fail("Failed to get surface capabilities! Error: %d", capResult); }
    
    u32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physicalDevice, ctx.surface, &formatCount, NULL);
    if (!formatCount) { fail("No surface formats found!"); }
    VkSurfaceFormatKHR* formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR)); vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physicalDevice, ctx.surface, &formatCount, formats);
    
    VkSurfaceFormatKHR selected_format = formats[0];
    
    for (u32 i = 0; i < formatCount; i++) {
        if (formats[i].format == PREFERRED_COLOR_FORMAT && formats[i].colorSpace == PREFERRED_COLOR_SPACE) {
            selected_format = formats[i];
            break;
        }
    }
    free(formats);
    
    u32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx.physicalDevice, ctx.surface, &presentModeCount, NULL);
    if (presentModeCount == 0) { fail("No present modes found!"); }
    VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    
    VkExtent2D extent = {W_WIDTH, W_HEIGHT};
    if (capabilities.currentExtent.width != 0xFFFFFFFF) {
        extent = capabilities.currentExtent;
    } else {
        extent.width = (u32)width;
        extent.height = (u32)height;
        if (extent.width < capabilities.minImageExtent.width)   { extent.width = capabilities.minImageExtent.width;   }
        if (extent.width > capabilities.maxImageExtent.width)   { extent.width = capabilities.maxImageExtent.width;   }
        if (extent.height < capabilities.minImageExtent.height) { extent.height = capabilities.minImageExtent.height; }
        if (extent.height > capabilities.maxImageExtent.height) { extent.height = capabilities.maxImageExtent.height; }
    }
    
    u32 imageCount = VK_IMAGE_COUNT;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    if (imageCount < capabilities.minImageCount) {
        imageCount = capabilities.minImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ctx.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = selected_format.format;
    createInfo.imageColorSpace = selected_format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = selectedPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    
    VkResult result = vkCreateSwapchainKHR(ctx.device, &createInfo, NULL, &ctx.swapChain);
    if (result != VKS) { fail("Failed to create swapchain! Error: %d", result); }

    u32 count;
    vkGetSwapchainImagesKHR(ctx.device, ctx.swapChain, &count, NULL);
    
    if (count > MAX_SWAPCHAIN_IMAGES) { fail("Swapchain has too many images!"); }
    
    ctx.swapChainImageCount = count;
    vkGetSwapchainImagesKHR(ctx.device, ctx.swapChain, &count, ctx.swapChainImages);
    
    ctx.swapChainImageFormat = createInfo.imageFormat;
    ctx.swapChainExtent = createInfo.imageExtent;
}
  
void create_image_views()
{
    for (u32 i = 0; i < ctx.swapChainImageCount; i++) {
        ctx.swapChainImageViews[i] = create_image_view(ctx.swapChainImages[i], ctx.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}
  
void create_render_pass()
{
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = ctx.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
  
    if (vkCreateRenderPass(ctx.device, &renderPassInfo, NULL, &ctx.renderPass) != VKS) {
        fail("Failed to create render pass!");
    }
}
 
void create_framebuffers()
{
    for (size_t i = 0; i < ctx.swapChainImageCount; i++) {
        VkImageView attachments[] = { ctx.swapChainImageViews[i] };
        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = ctx.renderPass;
        framebufferInfo.attachmentCount = (u32) (sizeof(attachments) / sizeof(attachments[0]));
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = ctx.swapChainExtent.width;
        framebufferInfo.height = ctx.swapChainExtent.height;
        framebufferInfo.layers = 1;
  
        if (vkCreateFramebuffer(ctx.device, &framebufferInfo, NULL, &ctx.swapChainFramebuffers[i]) != VKS) {
            fail("Failed to create framebuffer!");
        }
    }    
}


/* BUFFERS MANAGEMENT */

u32 find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(ctx.physicalDevice, &mem_properties);
    
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && 
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    fail("Failed to find suitable memory type!");
    return 1;
}

void create_buffers(
    u32 count,
    VkBuffer *buffers,
    VkDeviceMemory *memories,
    VkDeviceSize size,
    VkBufferUsageFlags usage_bit,
    VkMemoryPropertyFlags properties,
    const char *name)
{
    for (u32 i = 0; i < count; i++) {
        VkBufferCreateInfo buffer_info = {0};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage_bit;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(ctx.device, &buffer_info, NULL, &buffers[i]) != VKS) {
            fail("Failed to create [%s] : %u!", name, i);
        }
        
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(ctx.device, buffers[i], &mem_requirements);
        
        VkMemoryAllocateInfo alloc_info = {0};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);
        
        if (vkAllocateMemory(ctx.device, &alloc_info, NULL, &memories[i]) != VKS) {
            fail("Failed to allocate memory for [%s] : %u!", name, i);
        }
        
        vkBindBufferMemory(ctx.device, buffers[i], memories[i], 0);
    }
}

void create_descriptor_set_layout(VkDescriptorSetLayout *layout,
                                  VkDescriptorType type, const char *name)
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = type;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    ubo_layout_binding.pImmutableSamplers = NULL;
    
    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ubo_layout_binding;
    
    if (vkCreateDescriptorSetLayout(ctx.device, &layout_info, NULL, layout) != VKS) {
        fail("Failed to create descriptor set for [%s]!", name);
    }
}

void create_descriptor_pool(void)
{
    VkDescriptorPoolSize pool_sizes[2];
    
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = VK_IMAGE_COUNT;
    
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[1].descriptorCount = 2;
    
    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 2;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = VK_IMAGE_COUNT + 2;
    
    if (vkCreateDescriptorPool(ctx.device, &pool_info, NULL, &ctx.descriptor_pool) != VKS) {
        fail("Failed to create descriptor pool!");
    }
}

void create_descriptor_sets(
    u32 count,
    VkDescriptorSet *descriptor_sets,
    VkDescriptorSetLayout layout,
    VkBuffer *buffers,
    VkDeviceSize buffer_size,
    VkDescriptorType descriptor_type,
    const char *name)
{
    VkDescriptorSetLayout *layouts = malloc(count * sizeof(VkDescriptorSetLayout));
    for (u32 i = 0; i < count; i++) {
        layouts[i] = layout;
    }
    
    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ctx.descriptor_pool;
    alloc_info.descriptorSetCount = count;
    alloc_info.pSetLayouts = layouts;
    
    if (vkAllocateDescriptorSets(ctx.device, &alloc_info, descriptor_sets) != VKS) {
        fail("Failed to allocate descriptor sets for [%s]!", name);
    }
    
    for (u32 i = 0; i < count; i++) {
        VkDescriptorBufferInfo buffer_info = {0};
        buffer_info.buffer = buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = buffer_size;
        
        VkWriteDescriptorSet descriptor_write = {0};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = descriptor_type;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        
        vkUpdateDescriptorSets(ctx.device, 1, &descriptor_write, 0, NULL);
    }
    free(layouts);
}

void update_buffer(VkDeviceMemory memory, const void *data, VkDeviceSize size)
{
    void *mapped_data;
    vkMapMemory(ctx.device, memory, 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, size);
    vkUnmapMemory(ctx.device, memory);
}


/* GRAPHICS PIPELINE */

VkShaderModule createShaderModule(const u32 *code, u32 size)
{
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = code;
    VkShaderModule shaderModule;

    if (vkCreateShaderModule(ctx.device, &createInfo, NULL, &shaderModule) != VKS) {
        fail("Failed to create shader module!");
    }
    return shaderModule;
}

void create_graphics_pipeline()
{
    VkShaderModule vert_shader_m = createShaderModule(vert_spv, vert_size);
    VkShaderModule frag_shader_m = createShaderModule(frag_spv, frag_size);
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vert_shader_m;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = frag_shader_m;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)ctx.swapChainExtent.width;
    viewport.height = (f32)ctx.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D) {0, 0};
    scissor.extent = ctx.swapChainExtent;
    
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    
    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkDescriptorSetLayout layouts[] = {
        ctx.ubo_descriptor_set_layout,
        ctx.ssbo_descriptor_set_layout
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 2;
    pipeline_layout_info.pSetLayouts = layouts;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = NULL;
    
    if (vkCreatePipelineLayout(ctx.device, &pipeline_layout_info, NULL, &ctx.pipeline_layout) != VKS) {
        fail("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = ctx.pipeline_layout;
    pipelineInfo.renderPass = ctx.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    VkResult result = vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &ctx.graphicsPipeline);
    if (result != VKS) {
        fail("failed to create graphics pipeline! Error: %d", result);
    }
    vkDestroyShaderModule(ctx.device, vert_shader_m, NULL);
    vkDestroyShaderModule(ctx.device, frag_shader_m, NULL);
}


/* COMMAND MANAGEMENT */

void create_command_buffers()
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = VK_IMAGE_COUNT;
    
    VkResult result = vkAllocateCommandBuffers(ctx.device, &allocInfo, ctx.commandBuffers);
    if (result != VKS) {
      fail("failed to allocate command buffers! Error: %d\n", result);
    }
}

void create_command_pool()
{
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = ctx.DeviceQueueIndex;
    if (vkCreateCommandPool(ctx.device, &poolInfo, NULL, &ctx.commandPool) != VKS) {
      fail("Failed to create command pool!");
    }
}

void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VKS) {
        fail("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx.renderPass;
    renderPassInfo.framebuffer = ctx.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D) {0, 0};
    renderPassInfo.renderArea.extent = ctx.swapChainExtent;

    VkClearValue clearValues[1] = {};
    clearValues[0].color = (VkClearColorValue) {{0.0f, 0.0f, 0.0f, 1.0f}};

    renderPassInfo.clearValueCount = (uint32_t) (sizeof(clearValues) / sizeof(clearValues[0]));
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.graphicsPipeline);


    VkDescriptorSet sets[] = {
        ctx.ubo_descriptor_sets[ubo.image],
        ctx.ssbo_descriptor_set
    };

    vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                             ctx.pipeline_layout, 0, 2, sets, 0, NULL );
    
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)ctx.swapChainExtent.width;
    viewport.height = (f32)ctx.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = ctx.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VKS) {
        fail("failed to record command buffer!");
    }
}


/* SYNC & DRAW */

void create_sync_objects()
{
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < VK_IMAGE_COUNT; i++) {
        if (vkCreateSemaphore(ctx.device, &semaphoreInfo, NULL, &ctx.imageAvailableSemaphores[i]) != VKS ||
            vkCreateSemaphore(ctx.device, &semaphoreInfo, NULL, &ctx.renderFinishedSemaphores[i]) != VKS ||
            vkCreateSemaphore(ctx.device, &semaphoreInfo, NULL, &ctx.computeFinishedSemaphores[i]) != VKS ||
            vkCreateFence(ctx.device, &fenceInfo, NULL, &ctx.inFlightFences[i]) != VKS) {
            fail("Failed to create synchronization objects!");
        }
    }
}

void draw_frame()
{
    vkWaitForFences(ctx.device, 1, &ctx.inFlightFences[ubo.image], VK_TRUE, UINT64_MAX);
    vkResetFences(ctx.device, 1, &ctx.inFlightFences[ubo.image]);


    /* compute part */
    vkWaitForFences(ctx.device, 1, &ctx.compute_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx.device, 1, &ctx.compute_fence);
    
    record_compute_commands();
    

    VkSubmitInfo compute_submit = {0};
    compute_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    compute_submit.commandBufferCount = 1;
    compute_submit.pCommandBuffers = &ctx.compute_command_buffer;
    compute_submit.signalSemaphoreCount = 1;
    compute_submit.pSignalSemaphores = &ctx.computeFinishedSemaphores[ubo.image];

    vkQueueSubmit(ctx.queue, 1, &compute_submit, ctx.compute_fence);
    /* end compute */


    ubo.prev = ubo.curr;
    ubo.curr = glfwGetTime();


    mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(ctx.window);
    ubo.mouse.pos_x = state->pos_x;
    ubo.mouse.pos_y = state->pos_y;
    ubo.mouse.scroll = state->scroll;
    ubo.mouse.buttons = state->buttons;

    update_buffer(ctx.ubo_memories[ubo.image], &ubo, sizeof(ubo));


    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(ctx.device, ctx.swapChain, UINT64_MAX, 
                                            ctx.imageAvailableSemaphores[ubo.image], 
                                            VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR ) { 
        handle_window_resize();
        return;
    }
    
    vkResetCommandBuffer(ctx.commandBuffers[ubo.image], 0);
    record_command_buffer(ctx.commandBuffers[ubo.image], imageIndex);
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {
        ctx.imageAvailableSemaphores[ubo.image],
        ctx.computeFinishedSemaphores[ubo.image]
    };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
    };
    submitInfo.waitSemaphoreCount = 2;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx.commandBuffers[ubo.image];
    
    VkSemaphore signalSemaphores[] = {ctx.renderFinishedSemaphores[ubo.image]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    vkQueueSubmit(ctx.queue, 1, &submitInfo, ctx.inFlightFences[ubo.image]);
    
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {ctx.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(ctx.queue, &presentInfo);
     if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ctx.resize_request = 1;
        handle_window_resize();
     }
    
    if (ctx.resize_request) { handle_window_resize(); }
    ubo.image = (ubo.image + 1) % VK_IMAGE_COUNT;
    ubo.frame++;
}


/* LIFECYCLE MANAGEMENT */

void init_vulkan()
{
    create_instance();
    create_surface();
    pick_physical_device();
    create_logical_device();

    i32 x, y;
    glfwGetFramebufferSize(ctx.window, &x, &y);
    create_swap_chain(x, y);
    create_image_views();
    create_render_pass();
    create_framebuffers();

    /* Create Layouts for buffers */

    create_descriptor_set_layout(&ctx.ubo_descriptor_set_layout,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, "UBO layout");
    create_descriptor_set_layout(&ctx.ssbo_descriptor_set_layout,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, "SSBO layout");

    /* Create buffers */

    create_buffers(
            VK_IMAGE_COUNT,
            ctx.ubo_buffers, // Already a pointer
            ctx.ubo_memories,
            sizeof(ubo_data),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            "UBO");

    create_buffers(
            1,
            &ctx.ssbo_buffer,
            &ctx.ssbo_memory,
            sizeof(ssbo_data),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "SSBO");

    create_descriptor_pool();

    create_descriptor_sets(
            VK_IMAGE_COUNT,
            ctx.ubo_descriptor_sets,
            ctx.ubo_descriptor_set_layout,
            ctx.ubo_buffers,
            sizeof(ubo_data),
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            "UBO");

    create_descriptor_sets(
            1,
            &ctx.ssbo_descriptor_set,
            ctx.ssbo_descriptor_set_layout,
            &ctx.ssbo_buffer,
            sizeof(ssbo_data),
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            "SSBO");

    create_command_pool();

    create_compute_descriptor_set_layout();
    create_compute_descriptor_set();
    create_compute_pipeline();
    create_compute_command_buffer(); 

    create_graphics_pipeline();
    create_command_buffers();
    create_sync_objects();

    info("Vulkan initialized successfully");
    ubo.prev = glfwGetTime();
}

void cleanup_swap_chain()
{
    iterate(i, ctx.swapChainImageCount) {
        vkDestroyFramebuffer(ctx.device, ctx.swapChainFramebuffers[i], NULL);
        vkDestroyImageView(ctx.device, ctx.swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(ctx.device, ctx.swapChain, NULL);
}

void cleanup()
{
    cleanup_compute();
    cleanup_swap_chain();

    vkDestroyPipeline(ctx.device, ctx.graphicsPipeline, NULL);
    vkDestroyPipelineLayout(ctx.device, ctx.pipeline_layout, NULL);
    vkDestroyDescriptorPool(ctx.device, ctx.descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.ubo_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.ssbo_descriptor_set_layout, NULL);
    
    for (u32 i = 0; i < VK_IMAGE_COUNT; i++) {
        vkDestroyBuffer(ctx.device, ctx.ubo_buffers[i], NULL);
        vkFreeMemory(ctx.device, ctx.ubo_memories[i], NULL);

    }

    vkDestroyBuffer(ctx.device, ctx.ssbo_buffer, NULL);
    vkFreeMemory(ctx.device, ctx.ssbo_memory, NULL);

    vkDestroyCommandPool(ctx.device, ctx.commandPool, NULL);

    iterate(i, VK_IMAGE_COUNT) {
        vkDestroySemaphore(ctx.device, ctx.imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(ctx.device, ctx.renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(ctx.device, ctx.computeFinishedSemaphores[i], NULL);
        vkDestroyFence(ctx.device, ctx.inFlightFences[i], NULL);
    }

    vkDestroyRenderPass(ctx.device, ctx.renderPass, NULL);
    vkDestroyDevice(ctx.device, NULL);
    vkDestroySurfaceKHR(ctx.instance, ctx.surface, NULL);
    vkDestroyInstance(ctx.instance, NULL);
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
}

void handle_window_resize()
{
    i32 x = 0, y = 0;
    glfwGetFramebufferSize(ctx.window, &x, &y);
    
    while (!y + !x) {
        glfwGetFramebufferSize(ctx.window, &x, &y);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(ctx.device);

    ubo.x_size = (f32)x;
    ubo.y_size = (f32)y;

    cleanup_swap_chain();
    create_swap_chain(x, y);
    create_image_views();
    create_framebuffers();
    ctx.resize_request = 0;
}
