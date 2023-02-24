#include "vulkan_device.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

namespace vlk {

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
        vkDestroyInstance(m_instance, nullptr);
    }

    void VulkanDevice::_create_instance() {
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = m_window->get_title().c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;

        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        instance_create_info.enabledExtensionCount = glfw_extension_count;
        instance_create_info.ppEnabledExtensionNames = glfw_extensions;
        instance_create_info.enabledLayerCount = 0;

        if (vkCreateInstance(&instance_create_info, nullptr, &m_instance) != VK_SUCCESS) {
            std::cout << "failed to create vulkan instance\n";
            std::exit(-1);
        }

        std::cout << "successfully created vulkan instance with the extensions:\n";
        for (uint32_t i = 0; i < glfw_extension_count; i++) {
            std::cout << "\t* " << glfw_extensions[i] << "\n";
        }
    }

    void VulkanDevice::_setup_debug_messenger() {

    }

    void VulkanDevice::_pick_physical_device() {

    }

    void VulkanDevice::_create_logical_device() {

    }

    void VulkanDevice::_create_command_pool() {

    }

}