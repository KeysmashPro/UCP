/* vulkan.h */

#ifndef VULKAN_H
#define VULKAN_H

#include "callback.h"
#include "defines.h"

#define VKS VK_SUCCESS
#define VK_IMAGE_COUNT 3
#define MAX_SWAPCHAIN_IMAGES 8

typedef struct {
    u8 resize_request;

    GLFWwindow *window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    u32 DeviceQueueIndex;

    u32 swapChainImageCount;
    VkSwapchainKHR swapChain;
    VkImage swapChainImages[MAX_SWAPCHAIN_IMAGES];
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView swapChainImageViews[MAX_SWAPCHAIN_IMAGES];
    VkRenderPass renderPass;
    VkFramebuffer swapChainFramebuffers[MAX_SWAPCHAIN_IMAGES];
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[VK_IMAGE_COUNT];

    VkSemaphore imageAvailableSemaphores[VK_IMAGE_COUNT];
    VkSemaphore renderFinishedSemaphores[VK_IMAGE_COUNT];
    VkFence inFlightFences[VK_IMAGE_COUNT];

    VkBuffer ubo[VK_IMAGE_COUNT];
    VkDeviceMemory ubo_memory[VK_IMAGE_COUNT];

    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set[VK_IMAGE_COUNT];
    VkPipelineLayout pipeline_layout;
} vk_context;

typedef struct {
    f32 curr;
    f32 prev;
    u32 frame;
    u32 image;
    f32 x_size;
    f32 y_size;
    mouse_state mouse;
} ubo_data;


extern vk_context ctx;


void initWindow(void);
void createInstance(void);
void createSurface(void);
void pickPhysicalDevice(void);
void createLogicalDevice(void);

void createSwapChain(void);
VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, u32 miplevels);
void createImageViews(void);
void createRenderPass(void);

u32 find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties);
void create_uniform_buffers(void);
void create_descriptor_set_layout(void);
void create_descriptor_pool(void);
void create_descriptor_sets(void);

void update_uniform_buffer(uint32_t currentFrame);
VkShaderModule createShaderModule(const u32 *code, u32 size);
void createGraphicsPipeline(void);

void createCommandPool(void);
void createCommandBuffers(void);
void createSyncObjects(void);
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

void drawFrame(void);
void initVulkan(void);
void cleanup_swap_chain(void);
void cleanup(void);
void handle_window_resize(void);

#endif
