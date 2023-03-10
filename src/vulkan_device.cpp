#include "vulkan_device.hpp"

#include <GLFW/glfw3.h>
#include <iostream>     // std::cout, std::exit, std::...
#include <set>          // std::set
#include <limits>       // std::numeric_limits
#include <algorithm>    // std::clamp

namespace vlk {

#if defined(NDEBUG)
    bool VulkanDevice::m_enable_validation_layers = false;
    std::vector<const char*> VulkanDevice::m_validation_layers{};
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_call_back() { return VK_TRUE; }
#else
    bool VulkanDevice::m_enable_validation_layers = true;
    std::vector<const char*> VulkanDevice::m_validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_call_back(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {

        if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cout << callback_data->pMessage << "\n\n";

            // TODO: improve validation to be more specific with what objects it is affecting
        }
        else {
            return VK_TRUE;
        }

        return VK_FALSE;
    }
#endif

    std::vector<const char*> VulkanDevice::m_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    std::vector<const char*> VulkanDevice::get_required_extensions() {
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> required_extensions;
        for (uint32_t i = 0; i < glfw_extension_count; i++) {
            required_extensions.push_back(glfw_extensions[i]);
        }

        if (m_enable_validation_layers) {
            required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return required_extensions;
    }

    QueueFamilyIndices QueueFamilyIndices::query(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices{};

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        uint32_t i = 0;
        for (VkQueueFamilyProperties& property : queue_families) {
            if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family = i;
            }

            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
            if (present_support == VK_TRUE) {
                indices.present_family = i;
            }

            if (indices.supports_rendering()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapchainSupportDetails SwapchainSupportDetails::query(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        SwapchainSupportDetails support_details{};

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support_details.capabilities);

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        if (format_count != 0) {
            support_details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, support_details.formats.data());

            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
            if (present_mode_count != 0) {
                support_details.present_modes.resize(present_mode_count);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, support_details.present_modes.data());
            }
        }

        return support_details;
    }

    bool SwapchainSupportDetails::is_supported() const {
        return !formats.empty() && !present_modes.empty();
    }

    VkSurfaceFormatKHR SwapchainSupportDetails::choose_surface_format() {
        if (present_modes.size() == 0) {
            std::cout << "cannot choose surface format from SwapchainSupportDetails as there is nothing in the vector formats\n";
            std::exit(-1);
        }

        for (VkSurfaceFormatKHR& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR SwapchainSupportDetails::choose_present_mode() {
        if (present_modes.size() == 0) {
            std::cout << "cannot choose surface present mode from SwapchainSupportDetails as there is nothing in the vector present_modes\n";
            std::exit(-1);
        }

        for (VkPresentModeKHR& present_mode : present_modes) {
            if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapchainSupportDetails::choose_swapchain_extent(Window* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window->get_internal_window(), &width, &height);
        VkExtent2D actual_size = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        actual_size.width = std::clamp(actual_size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_size.height = std::clamp(actual_size.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_size;
    }

    VulkanDevice::VulkanDevice(Window* window) : m_window(window) {
        print_extension_support();

        _init_instance();
        _init_debug_manager();

        m_window->init_surface(m_instance);

        _init_physical_device();
        _init_logical_device();
        _init_swapchain();
        _init_swapchain_images();
    }

    VulkanDevice::~VulkanDevice() {
#if !defined(NDEBUG)
        if (m_enable_validation_layers) {
            auto destroy_debug_utils_messenger_ext_func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (destroy_debug_utils_messenger_ext_func != nullptr) {
                destroy_debug_utils_messenger_ext_func(m_instance, m_debug_messenger, nullptr);
            }
            else {
                std::cout << "vkDestroyDebugMessengerEXT function doesn't exist\n";
            }
        }
#endif
        for (VkImageView& image_view : m_swapchain_image_views) {
            vkDestroyImageView(m_device, image_view, nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        vkDestroyDevice(m_device, nullptr);
        m_window->destroy_surface(m_instance);
        vkDestroyInstance(m_instance, nullptr);
    }

    void VulkanDevice::print_extension_support() const {
        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        std::cout << "list of available vulkan extensions that your devices supports:\n";
        for (const VkExtensionProperties& extension : extensions) {
            std::cout << "\t* (" << extension.specVersion << "):\t" << extension.extensionName << "\n";
        }
    }

    void VulkanDevice::_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        create_info.pfnUserCallback = debug_call_back;
    }

    bool VulkanDevice::_check_validation_layer_support() {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        std::cout << "checking available validation layers\n";
        bool found_all_layers = true;
        for (const char* layer_name : m_validation_layers) {
            bool found_layer = false;
            size_t layer_length = std::strlen(layer_name);

            for (const VkLayerProperties& layer_property : available_layers) {
                if (std::strncmp(layer_name, layer_property.layerName, layer_length) == 0) {
                    found_layer = true;
                    std::cout << "\t* " << layer_name << " found\n";
                    break;
                }
            }

            if (!found_layer) {
                std::cout << "failed to find the validation layer: " << layer_name << " in available layers\n";
                found_all_layers = false;
            }
        }

        return found_all_layers;
    }

    bool VulkanDevice::_check_physical_device_required_extensions_support(VkPhysicalDevice physical_device) {
        uint32_t device_extension_count = 0;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);
        std::vector<VkExtensionProperties> device_extensions(device_extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.data());

        std::cout << "\t  available physical device extensions device extensions:\n";
        for (VkExtensionProperties& extensions : device_extensions) {
            std::cout << "\t\t* " << extensions.extensionName << "\n";
        }

        for (const char* required_extension : m_device_extensions) {
            bool found = false;
            size_t required_extension_length = std::strlen(required_extension);
            for (VkExtensionProperties& extension : device_extensions) {
                if (std::strncmp(required_extension, extension.extensionName, required_extension_length) == 0) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }

        
       return true;
    }

    void VulkanDevice::_init_instance() {
        if (m_enable_validation_layers && !_check_validation_layer_support()) {
            std::cout << "validation layer requested, but not available\n";
            std::exit(-1);
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = m_window->get_title().c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        std::vector<const char*> required_extensions = get_required_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
        create_info.ppEnabledExtensionNames = required_extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (m_enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();

            _populate_debug_messenger_create_info(debug_create_info);
            create_info.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_create_info);
        }
        else {
            create_info.enabledLayerCount = 0;
            create_info.ppEnabledExtensionNames = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
            std::cout << "failed to create vulkan instance\n";
            std::exit(-1);
        }

        std::cout << "successfully initialized vulkan instance with the extensions:\n";
        for (const char* extension : required_extensions) {
            std::cout << "\t* " << extension << "\n";
        }
    }

    void VulkanDevice::_init_debug_manager() {
#if !defined(NDEBUG)
        if (!m_enable_validation_layers) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT create_info;
        _populate_debug_messenger_create_info(create_info);

        // getting the vkCreateDebugUtilsMessengerEXT function because its and ext therefore its not automatically loaded
        auto create_debug_utils_messenger_ext_func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (create_debug_utils_messenger_ext_func != nullptr) {
            if (create_debug_utils_messenger_ext_func(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS) {
                std::cout << "failed to call vkCreateDebugUtilsMessengerEXT pointer function\n";
                std::exit(-1);
            }
        }
        else {
            std::cout << "vkCreateDebugUtilsMessengerEXT function doesn't exit\n";
            std::exit(-1);
        }

        std::cout << "successfully initialized vulkan debug messenger ext\n";
#endif
    }

    void VulkanDevice::_init_physical_device() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

        if (device_count == 0) {
            std::cout << "failed to find any physical devices capable of vulkan rendering\n";
            std::exit(-1);
        }

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(m_instance, &device_count, physical_devices.data());

        // checking if the device is suitable
        std::cout << "physical devices on system:\n";
        int score_to_beat = 0;
        VkPhysicalDeviceProperties physical_device_properties;
        for (VkPhysicalDevice physical_device : physical_devices) {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;

            vkGetPhysicalDeviceProperties(physical_device, &properties);
            vkGetPhysicalDeviceFeatures(physical_device, &features);

            std::cout << "\t* " << properties.deviceName << "\n";

            if (_check_physical_device_required_extensions_support(physical_device)) {
                QueueFamilyIndices indices = QueueFamilyIndices::query(physical_device, m_window->get_surface());
                if (!indices.supports_rendering()) {
                    continue;
                }

                SwapchainSupportDetails swapchain_support = SwapchainSupportDetails::query(physical_device, m_window->get_surface());
                if (!swapchain_support.is_supported()) {
                    continue;
                }

                if (features.geometryShader) {
                    int score = 0;
                    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        score += 1000;
                    }
                    score += properties.limits.maxImageDimension2D;

                    if (score > score_to_beat) {
                        score_to_beat = score;
                        m_physical_device = physical_device;
                        physical_device_properties = properties;
                        m_physical_device_features = features;
                    }
                }
            }
        }

        if (m_physical_device == nullptr) {
            std::cout << "something serious has happened when picking a physical device, cause this should never be called\n";
            std::exit(-1);
        }

        std::cout << "chosen physical device: " << physical_device_properties.deviceName << ", extensions enabled:\n";
        for (const char* extension_name : m_device_extensions) {
            std::cout << "\t* " << extension_name << "\n";
        }
    }

    void VulkanDevice::_init_logical_device() {
        QueueFamilyIndices indices = QueueFamilyIndices::query(m_physical_device, m_window->get_surface());

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        std::set<uint32_t> queue_create_info_ids = { indices.graphics_family.value(), indices.present_family.value() };

        for (uint32_t queue_family_id : queue_create_info_ids) {
            VkDeviceQueueCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_info.pQueuePriorities = &indices.priority;
            create_info.queueFamilyIndex = queue_family_id;
            create_info.queueCount = 1;
            queue_create_infos.push_back(create_info);
        }

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pEnabledFeatures = &m_physical_device_features;
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());

        create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
        create_info.ppEnabledExtensionNames = m_device_extensions.data();

        // for older version of vulkan, newer versions will ignore this parameters
        if (m_enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();
        }
        else {
            create_info.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS) {
            std::cout << "failed to create logical device\n";
            std::exit(-1);
        }

        vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
        vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);

        std::cout << "successfully initialized vulkan logical device\n";
    }

    void VulkanDevice::_init_swapchain() {
        SwapchainSupportDetails support = SwapchainSupportDetails::query(m_physical_device, m_window->get_surface());

        m_swapchain_surface_format = support.choose_surface_format();
        m_swapchain_present_mode = support.choose_present_mode();
        m_swapchain_extent = support.choose_swapchain_extent(m_window);

        uint32_t image_count = support.capabilities.minImageCount + 1; // recommended to go at least one over the minimum
        if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
            image_count = support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = m_window->get_surface();
        create_info.minImageCount = image_count;
        create_info.imageFormat = m_swapchain_surface_format.format;
        create_info.imageExtent = m_swapchain_extent;
        create_info.imageArrayLayers = 1; // this should always be 1, unless trying to create stereoscopic 3D applications
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 

        QueueFamilyIndices indices = QueueFamilyIndices::query(m_physical_device, m_window->get_surface());
        uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

        if (indices.graphics_family != indices.present_family) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        }
        else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = support.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        create_info.presentMode = m_swapchain_present_mode;
        create_info.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS) {
            std::cout << "failed to create vulkan swapchain\n";
            std::exit(-1);
        }

        std::cout << "successfully initialized vulkan swapchain\n";
    }

    void VulkanDevice::_init_swapchain_images() {
        uint32_t swapchain_image_count = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);
        m_swapchain_images.resize(swapchain_image_count);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, m_swapchain_images.data());

        m_swapchain_image_views.resize(m_swapchain_images.size());
        for (size_t i = 0; i < m_swapchain_images.size(); i++) {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = m_swapchain_images[i];
            create_info.format = m_swapchain_surface_format.format;
            create_info.viewType =  VK_IMAGE_VIEW_TYPE_2D;

            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            // TODO: learn mipmap stuff ...
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device, &create_info, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS) {
                std::cout << "failed to create image views\n";
                std::exit(-1);
            }
        }

        std::cout << "successfully created swapchain image views\n";
    }

}