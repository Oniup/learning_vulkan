#ifndef __VULKAN_DEVICE_HPP__
#define __VULKAN_DEVICE_HPP__

#include "window.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace vlk {

    class VulkanDevice {
    public:
        static std::vector<const char*> get_required_extensions();

        VulkanDevice(Window* window);
        ~VulkanDevice() = default;

        void print_extension_support() const;
        void terminate();

    private:
        void _create_instance();
        void _setup_debug_messenger();
        void _pick_physical_device();
        void _create_logical_device();
        void _create_command_pool();

        void _populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
        bool _check_validation_layer_support();

        static bool m_enable_validation_layers;
        static std::vector<const char*> m_validation_layers;

        Window* m_window;
        VkInstance m_instance;

#if !defined(NDEBUG)
        VkDebugUtilsMessengerEXT m_debug_messenger;
#endif 
    };

}

#endif // __VULKAN_DEVICE_HPP__