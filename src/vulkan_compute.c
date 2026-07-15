/* vulkan_compute.c */

#include "defines.h"
#include "callback.h"
#include "vulkan_header.h"
#include "../lib/volk/volk.h"

#include "shaders/shaderdump.h"


void create_compute_descriptor_set_layout(void)
{
    VkDescriptorSetLayoutBinding ssbo_binding = {0};
    ssbo_binding.binding = 0;
    ssbo_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssbo_binding.descriptorCount = 1;
    ssbo_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &ssbo_binding;
    
    if (vkCreateDescriptorSetLayout(ctx.device, &layout_info, NULL, 
        &ctx.compute_descriptor_set_layout) != VKS) {
        fail("Failed to create compute descriptor set layout!");
    }
}

void create_compute_descriptor_set(void)
{
    create_descriptor_sets(
        1,
        &ctx.compute_descriptor_set,
        ctx.compute_descriptor_set_layout,
        &ctx.ssbo_buffer,
        sizeof(ssbo_data),
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        "Compute SSBO"
    );
}

void create_compute_pipeline(void)
{
    VkShaderModule compute_shader = createShaderModule(comp_spv, comp_size);
    
    VkPipelineShaderStageCreateInfo shader_stage = {0};
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage.module = compute_shader;
    shader_stage.pName = "main";

    VkDescriptorSetLayout layouts[] = {
        ctx.compute_descriptor_set_layout,
        ctx.ubo_descriptor_set_layout
    };
    
    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = layouts;
    
    if (vkCreatePipelineLayout(ctx.device, &pipeline_layout_info, NULL, 
        &ctx.compute_pipeline_layout) != VKS) {
        fail("Failed to create compute pipeline layout!");
    }
    
    VkComputePipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = shader_stage;
    pipeline_info.layout = ctx.compute_pipeline_layout;
    
    if (vkCreateComputePipelines(ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, 
        &ctx.compute_pipeline) != VKS) {
        fail("Failed to create compute pipeline!");
    }
    
    vkDestroyShaderModule(ctx.device, compute_shader, NULL);
}

void create_compute_command_buffer(void)
{
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = ctx.commandPool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;
    
    if (vkAllocateCommandBuffers(ctx.device, &alloc_info, &ctx.compute_command_buffer) != VKS) {
        fail("Failed to allocate compute command buffer!");
    }
    
    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(ctx.device, &fence_info, NULL, &ctx.compute_fence);
}

void record_compute_commands(void)
{
    vkResetCommandBuffer(ctx.compute_command_buffer, 0);
    
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkBeginCommandBuffer(ctx.compute_command_buffer, &begin_info);
    
    vkCmdBindPipeline(ctx.compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
                      ctx.compute_pipeline);
    
    VkDescriptorSet sets[] = {
        ctx.compute_descriptor_set,
        ctx.ubo_descriptor_sets[ubo.image]
    };
    vkCmdBindDescriptorSets(ctx.compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        ctx.compute_pipeline_layout, 0, 2, sets, 0, NULL);
    
    u32 workgroup_count = (sizeof(ssbo_data) / sizeof(f32) + 255) / 256;
    vkCmdDispatch(ctx.compute_command_buffer, workgroup_count, 1, 1);
    
    vkEndCommandBuffer(ctx.compute_command_buffer);
}

void cleanup_compute(void)
{
    vkDestroyPipeline(ctx.device, ctx.compute_pipeline, NULL);
    vkDestroyPipelineLayout(ctx.device, ctx.compute_pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.compute_descriptor_set_layout, NULL);
    vkDestroyFence(ctx.device, ctx.compute_fence, NULL);
}
