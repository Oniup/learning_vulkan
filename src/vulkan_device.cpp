#include "vulkan_device.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <set>

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

    VulkanDevice::VulkanDevice(Window* window) : m_window(window) {
        print_extension_support();

        _init_instance();
        _init_debug_manager();

        m_window->init_surface(m_instance);

        _init_physical_device();
        _init_logical_device();
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

    void VulkanDevice::terminate() {
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
        vkDestroyDevice(m_device, nullptr);
        m_window->destroy_surface(m_instance);
        vkDestroyInstance(m_instance, nullptr);
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

    VulkanDeviceQueueFamilyIndices VulkanDevice::_find_rendering_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        VulkanDeviceQueueFamilyIndices indices{};

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

            if (indices.is_rendering_complete()) {
                break;
            }

            i++;
        }

        return indices;
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
        for (VkPhysicalDevice device : physical_devices) {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;

            vkGetPhysicalDeviceProperties(device, &properties);
            vkGetPhysicalDeviceFeatures(device, &features);

            std::cout << "\t* " << properties.deviceName << "\n";

            if (_check_physical_device_required_extensions_support(device)) {
                if (features.geometryShader) {
                    int score = 0;
                    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        score += 1000;
                    }
                    score += properties.limits.maxImageDimension2D;

                    if (score > score_to_beat) {
                        score_to_beat = score;
                        m_physical_device = device;
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
        VulkanDeviceQueueFamilyIndices indices = _find_rendering_queue_families(m_physical_device, m_window->get_surface());

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

}