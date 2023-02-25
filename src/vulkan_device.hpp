#ifndef __VULKAN_DEVICE_HPP__
#define __VULKAN_DEVICE_HPP__

#include "window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace vlk {

    class VulkanDevice {
    public:
        static std::vector<const char*> get_required_extensions();

        VulkanDevice(Window* window);
        ~VulkanDevice() = default;

        void print_extension_support() const;
        void terminate();

    private:
        struct QueueFamilyIndices {
            float priority{ 1.0f };
            std::optional<uint32_t> graphics_family{};

            inline bool is_complete() {
                return graphics_family.has_value();
            }
        };

        // initialization functions
        void _init_instance();
        void _init_debug_manager();
        void _init_physical_device();
        void _init_logical_device();

        void _populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
        bool _check_validation_layer_support();
        QueueFamilyIndices _find_queue_families(VkPhysicalDevice physical_device);

        static bool m_enable_validation_layers;
        static std::vector<const char*> m_validation_layers;

        Window* m_window{ nullptr };
        VkInstance m_instance{ nullptr };
        VkPhysicalDevice m_physical_device{ nullptr };
        VkPhysicalDeviceFeatures m_physical_device_features;
        VkDevice m_device{ nullptr };

#if !defined(NDEBUG)
        VkDebugUtilsMessengerEXT m_debug_messenger{ nullptr };
#endif 
    };

}

#endif // __VULKAN_DEVICE_HPP__