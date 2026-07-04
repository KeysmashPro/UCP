/* Vulkan.c */

#include "defines.h"
#include "callback.h"
#include "vulkan_header.h"

#include "vulkan_init.c"
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
    VkSurfaceFormatKHR* formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.physicalDevice, ctx.surface, &formatCount, formats);
    
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
    
    VkPresentModeKHR* presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx.physicalDevice, ctx.surface, &presentModeCount, presentModes);
    
    VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    free(presentModes);
    
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


/* BUFFER MANAGEMENT */

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
}

void create_uniform_bufers()
{
    VkDeviceSize bufferSize = sizeof(ubo_data);
    
    for (size_t i = 0; i < VK_IMAGE_COUNT; i++) {
        VkBufferCreateInfo bufferInfo = {0};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(ctx.device, &bufferInfo, NULL, &ctx.ubo[i]) != VKS) {
            fail("Failed to create uniform buffer %zu!", i);
        }
        
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(ctx.device, ctx.ubo[i], &mem_requirements);
        
        VkMemoryAllocateInfo allocInfo = {0};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = mem_requirements.size;
        allocInfo.memoryTypeIndex = find_memory_type(
            mem_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        
        if (vkAllocateMemory(ctx.device, &allocInfo, NULL, &ctx.ubo_memory[i]) != VKS) {
            fail("Failed to allocate uniform buffer memory %zu!", i);
        }
        
        vkBindBufferMemory(ctx.device, ctx.ubo[i], ctx.ubo_memory[i], 0);
    }
    
    info("Created %d uniform buffers", VK_IMAGE_COUNT);
}

void create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    
    if (vkCreateDescriptorSetLayout(ctx.device, &layoutInfo, NULL, &ctx.descriptor_set_layout) != VKS) {
        fail("Failed to create descriptor set layout!");
    }
}

void create_descriptor_pool()
{
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = VK_IMAGE_COUNT;
    
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = VK_IMAGE_COUNT;
    
    if (vkCreateDescriptorPool(ctx.device, &poolInfo, NULL, &ctx.descriptor_pool) != VKS) {
        fail("Failed to create descriptor pool!");
    }
}

void create_descriptor_sets()
{
    VkDescriptorSetLayout layouts[VK_IMAGE_COUNT];
    for (size_t i = 0; i < VK_IMAGE_COUNT; i++) {
        layouts[i] = ctx.descriptor_set_layout;
    }
    
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = ctx.descriptor_pool;
    allocInfo.descriptorSetCount = VK_IMAGE_COUNT;
    allocInfo.pSetLayouts = layouts;
    
    if (vkAllocateDescriptorSets(ctx.device, &allocInfo, ctx.descriptor_set) != VKS) {
        fail("Failed to allocate descriptor sets!");
    }
    
    for (size_t i = 0; i < VK_IMAGE_COUNT; i++) {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = ctx.ubo[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ubo_data);
        
        VkWriteDescriptorSet descriptorWrite = {0};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = ctx.descriptor_set[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        
        vkUpdateDescriptorSets(ctx.device, 1, &descriptorWrite, 0, NULL);
    }
    
    info("Created %d descriptor sets", VK_IMAGE_COUNT);
}

void update_uniform_buffer(uint32_t currentFrame)
{
    void* data;
    vkMapMemory(ctx.device, ctx.ubo_memory[currentFrame], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(ctx.device, ctx.ubo_memory[currentFrame]);
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

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &ctx.descriptor_set_layout;
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

    vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            ctx.pipeline_layout, 0, 1, &ctx.descriptor_set[ubo.image], 0, NULL
    );
    
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
            vkCreateFence(ctx.device, &fenceInfo, NULL, &ctx.inFlightFences[i]) != VKS) {
            fail("Failed to create synchronization objects!");
        }
    }
}

void draw_frame()
{
    vkWaitForFences(ctx.device, 1, &ctx.inFlightFences[ubo.image], VK_TRUE, UINT64_MAX);
    vkResetFences(ctx.device, 1, &ctx.inFlightFences[ubo.image]);
    ubo.prev = ubo.curr;
    ubo.curr = glfwGetTime();

    mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(ctx.window);
    ubo.mouse.pos_x = state->pos_x;
    ubo.mouse.pos_y = state->pos_y;

    update_uniform_buffer(ubo.image);

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
    
    VkSemaphore waitSemaphores[] = {ctx.imageAvailableSemaphores[ubo.image]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
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

    create_descriptor_set_layout();
    create_uniform_bufers();
    create_descriptor_pool();
    create_descriptor_sets();

    create_graphics_pipeline();
    create_command_pool();
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
    cleanup_swap_chain();
    vkDestroyPipeline(ctx.device, ctx.graphicsPipeline, NULL);

    vkDestroyPipelineLayout(ctx.device, ctx.pipeline_layout, NULL);
    vkDestroyDescriptorPool(ctx.device, ctx.descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.descriptor_set_layout, NULL);
    
    for (u32 i = 0; i < VK_IMAGE_COUNT; i++) {
        vkDestroyBuffer(ctx.device, ctx.ubo[i], NULL);
        vkFreeMemory(ctx.device, ctx.ubo_memory[i], NULL);
    }

    vkDestroyCommandPool(ctx.device, ctx.commandPool, NULL);
    
    iterate(i, VK_IMAGE_COUNT) {
        vkDestroySemaphore(ctx.device, ctx.imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(ctx.device, ctx.renderFinishedSemaphores[i], NULL);
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
