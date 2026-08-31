/* vulkan.h */

#ifndef VULKAN_H
#define VULKAN_H

#include "callback.h"
#include "defines.h"

#define VKS VK_SUCCESS
#define VK_IMAGE_COUNT 3
#define MAX_SWAPCHAIN_IMAGES 8

 
typedef struct {
    /* State */
    mouse_state mouse;
    u8 resize_request;

    /* Device */
    GLFWwindow *window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    u32 DeviceQueueIndex;

    /* Swapchain */
    u32 swapChainImageCount;
    VkSwapchainKHR swapChain;
    VkImage swapChainImages[MAX_SWAPCHAIN_IMAGES];
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView swapChainImageViews[MAX_SWAPCHAIN_IMAGES];

    /* Render */
    VkRenderPass renderPass;
    VkFramebuffer swapChainFramebuffers[MAX_SWAPCHAIN_IMAGES];
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipeline_layout;

    /* Comand buffers */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[VK_IMAGE_COUNT];

    /* Synchronization */
    VkSemaphore imageAvailableSemaphores[VK_IMAGE_COUNT];
    VkSemaphore renderFinishedSemaphores[VK_IMAGE_COUNT];
    VkSemaphore computeFinishedSemaphores[VK_IMAGE_COUNT];
    VkFence inFlightFences[VK_IMAGE_COUNT];
    
    /* Descriptor pool */
    VkDescriptorPool descriptor_pool;

    /* UBO */
    VkBuffer ubo_buffers[VK_IMAGE_COUNT];
    VkDeviceMemory ubo_memories[VK_IMAGE_COUNT];
    VkDescriptorSetLayout ubo_descriptor_set_layout;
    VkDescriptorSet ubo_descriptor_sets[VK_IMAGE_COUNT];
    
    /* SSBO */
    VkBuffer ssbo_buffer;
    VkDeviceMemory ssbo_memory;
    VkDescriptorSetLayout ssbo_descriptor_set_layout;
    VkDescriptorSet ssbo_descriptor_set;

    VkBuffer ssbo_staging_buffer;
    VkDeviceMemory ssbo_staging_memory;
    
    /* Compute */
    VkPipeline compute_pipeline;
    VkPipelineLayout compute_pipeline_layout;
    VkDescriptorSetLayout compute_descriptor_set_layout;
    VkDescriptorSet compute_descriptor_set;
    VkCommandBuffer compute_command_buffer;
    VkFence compute_fence;

} vk_context;

typedef struct {
    u32 string;
    u32 vertex;
    u32 object;
} ssbo_offset;

typedef struct {
    f32 curr;
    f32 prev;
    u32 frame;
    u32 image;
    f32 x_size;
    f32 y_size;
    mouse_state mouse;
    ssbo_offset offset;
} ubo_data;

typedef struct {
    f32 data[1024];
} ssbo_data;


/* variables */

extern vk_context ctx;
extern ubo_data ubo;
extern ssbo_data ssbo;


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
void create_buffers(
    u32 count,
    VkBuffer *buffers,
    VkDeviceMemory *memories,
    VkDeviceSize size,
    VkBufferUsageFlags usage_bit,
    VkMemoryPropertyFlags properties,
    const char *name);

void create_descriptor_set_layout(VkDescriptorSetLayout *layout, VkDescriptorType type, const char *name);
void create_descriptor_pool(void);
void create_descriptor_sets(
    u32 count,
    VkDescriptorSet *descriptor_sets,
    VkDescriptorSetLayout layout,
    VkBuffer *buffers,
    VkDeviceSize buffer_size,
    VkDescriptorType descriptor_type,
    const char *name);

void update_uniform_buffer(uint32_t currentFrame);
void update_buffer(VkDeviceMemory memory, const void *data, VkDeviceSize size);
VkShaderModule createShaderModule(const u32 *code, u32 size);
void createGraphicsPipeline(void);

void createCommandPool(void);
void createCommandBuffers(void);
void createSyncObjects(void);
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

void drawFrame(void);
void initVulkan(void);
void cleanup_compute(void);
void cleanup_swap_chain(void);
void cleanup(void);
void handle_window_resize(void);

/* Compute */

void create_compute_descriptor_set(void);;
void create_compute_pipeline(void);
void create_compute_command_buffer(void);
void record_compute_commands(void);

#endif
