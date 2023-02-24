#ifndef __VULKAN_DEVICE_HPP__
#define __VULKAN_DEVICE_HPP__

#include "window.hpp"
#include <vulkan/vulkan.h>

namespace vlk {

    class VulkanDevice {
    public:
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

        Window* m_window { nullptr };
        VkInstance m_instance;
    };

}

#endif // __VULKAN_DEVICE_HPP__