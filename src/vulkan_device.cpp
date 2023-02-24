#include "vulkan_device.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

namespace vlk {

#if defined(NDEBUG)
    bool VulkanDevice::m_enable_validation_layers = false;
    std::vector<const char*> VulkanDevice::m_validation_layers = {};
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
            std::cout << "validation layer [ ";
            std::cout << message_type && VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ? "general " : "";
            std::cout << message_type && VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ? "validation " : "";
            std::cout << message_type && VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ? "performance " : "";
            std::cout << message_type && VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT ? "device_address_binding " : "";
            std::cout << message_type && VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT ? "flag_bits_max_enum " : "";
            std::cout << "]: " << callback_data->pMessage << "\n";

            // TODO: improve validation to be more specific with what objects it is affecting
        }
        else {
            return VK_TRUE;
        }

        return VK_FALSE;
    }
#endif

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

        _create_instance();
        _setup_debug_messenger();
        _pick_physical_device();
        _create_logical_device();
        _create_command_pool();
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
        if (!m_enable_validation_layers) {
            auto destroy_debug_utils_messenger_ext_func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (destroy_debug_utils_messenger_ext_func != nullptr) {
                destroy_debug_utils_messenger_ext_func(m_instance, m_debug_messenger, nullptr);
            }
            else {
                std::cout << "vkDestroyDebugMessengerEXT function doesn't exist\n";
            }
        }
#endif

        vkDestroyInstance(m_instance, nullptr);
    }

    void VulkanDevice::_create_instance() {
        if (m_enable_validation_layers && !_check_validation_layer_support()) {
            std::cout << "validation layer requested, but not available\n";
            std::exit(-1);
        }

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = m_window->get_title().c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        std::vector<const char*> required_extensions = get_required_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
        create_info.ppEnabledExtensionNames = required_extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
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

        std::cout << "successfully created vulkan instance with the extensions:\n";
        for (const char* extension : required_extensions) {
            std::cout << "\t* " << extension << "\n";
        }
    }

    void VulkanDevice::_setup_debug_messenger() {
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
#endif
    }

    void VulkanDevice::_pick_physical_device() {

    }

    void VulkanDevice::_create_logical_device() {

    }

    void VulkanDevice::_create_command_pool() {

    }

    void VulkanDevice::_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        create_info.pfnUserCallback = debug_call_back;
        create_info.pUserData = nullptr;
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

}