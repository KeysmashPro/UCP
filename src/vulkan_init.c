/* init_vulkan.c */

#include "defines.h"
#include "callback.h"
#include "vulkan_header.h"

#include "../lib/volk/volk.h"

/* variables */

char *WIN_NAME = "mpc";
u64 W_WIDTH  = 720;
u64 W_HEIGHT = 480;

VkFormat PREFERRED_COLOR_FORMAT = VK_FORMAT_B8G8R8A8_SRGB;
VkColorSpaceKHR PREFERRED_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

vk_context ctx = {0};
ubo_data ubo   = {0};

/* DEVICE & INSTANCE CREATION */

void init_window()
{
    if (volkInitialize() != VKS) { fail("Error: Vulkan loader not found on this system!"); }
    glfwInitVulkanLoader(vkGetInstanceProcAddr);

    if(!glfwInit()) { fail("Failed to glfwInit!"); }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    ctx.window = glfwCreateWindow(W_WIDTH, W_HEIGHT, WIN_NAME, NULL, NULL);
    glfwSetFramebufferSizeCallback(ctx.window, framebuffer_resize_callback);
    if (!ctx.window) { glfwTerminate(); fail("Failed to create window!"); }
    glfwShowWindow(ctx.window);

    i32 x = 0, y = 0;
    glfwGetFramebufferSize(ctx.window, &x, &y);
    ubo.x_size = (f32)x;
    ubo.y_size = (f32)y;
}

void create_instance()
{
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "mpc";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    app_info.pEngineName = NULL;
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    u32 glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensions) { fail("Failed to get GLFW required extensions!"); }

    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    u32 layerCount = 0;

#ifdef DEBUG
    layerCount = 1; 
    info("Validation layers requested");
#endif

    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &app_info;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = validationLayers;

    VkResult instance_result = vkCreateInstance(&createInfo, NULL, &ctx.instance);
    if (instance_result != VKS) {
        fail("Vulkan instance creation failed! Error code: %d", instance_result);
    }
    volkLoadInstance(ctx.instance);
}

void create_surface()
{
    VkResult res = glfwCreateWindowSurface(ctx.instance, ctx.window, NULL, &ctx.surface);
    if (res != VKS) { fail("failed to create window surface! VkResult = %d", res); }
}

void pick_physical_device()
{
    u32 deviceCount = 0;
    VkResult res = vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, NULL);
    if (res != VKS) { fail("Fail to enumerate physical devices! Error code: %d", res); }
    if (!deviceCount) { fail("Found 0 devices with Vulkan support!\n"); }

    VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(ctx.instance, &deviceCount, devices);
    ctx.physicalDevice = devices[0];
    free(devices);
    
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(ctx.physicalDevice, &device_properties);
    info("Selected GPU: %s", device_properties.deviceName);
}

void create_logical_device()
{
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.physicalDevice, &queueFamilyCount, queueFamilies);
    
    u32 selectedIndex = UINT32_MAX;
    for (u32 i = 0; i < queueFamilyCount; i++) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        VkBool32 presentSupport = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(ctx.physicalDevice, i, ctx.surface, &presentSupport);
        if (presentSupport) {
          selectedIndex = i;
          info("Selected queue family %u (graphics + present)", i);
          break;
        }
      }
    }
    free(queueFamilies);
    
    if (selectedIndex == UINT32_MAX) { fail("No suitable queue family found!"); }
    
    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = selectedIndex;
    queueCreateInfo.queueCount = 1;
    f32 queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    // Add feature structure for shaderDrawParameters
    VkPhysicalDeviceVulkan11Features vulkan11Features = {0};
    vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vulkan11Features.shaderDrawParameters = VK_TRUE;
    
    const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
    u32 layerCount = 0;
    
    #ifdef DEBUG
    layerCount = 1;
    #endif
    
    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.enabledExtensionCount = 1;
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = layerCount > 0 ? validationLayers : NULL;
    createInfo.pNext = &vulkan11Features;
    
    VkResult result = vkCreateDevice(ctx.physicalDevice, &createInfo, NULL, &ctx.device);
    if (result != VKS) { fail("Failed to create logical device! Error: %d", result); }
    vkGetDeviceQueue(ctx.device, selectedIndex, 0, &ctx.queue);
    ctx.DeviceQueueIndex = selectedIndex;
}


/* */

VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, u32 miplevels)
{
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = miplevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.aspectMask = aspect_flags;
  
    VkImageView image_view;
    if (vkCreateImageView(ctx.device, &viewInfo, NULL, &image_view) != VKS) { fail("Failed to create image view!"); }
    return image_view;
}

