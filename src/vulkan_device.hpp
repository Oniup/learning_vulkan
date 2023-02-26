#ifndef __VULKAN_DEVICE_HPP__
#define __VULKAN_DEVICE_HPP__

#include "window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace vlk {

    struct VulkanDeviceQueueFamilyIndices {
        float priority{ 1.0f };
        std::optional<uint32_t> graphics_family{};
        std::optional<uint32_t> present_family{};

        inline bool is_rendering_complete() {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    class VulkanDevice {
    public:
        static std::vector<const char*> get_required_extensions();

        VulkanDevice(Window* window);
        ~VulkanDevice() = default;

        inline VkQueue get_graphics_queue() { return m_graphics_queue; }
        inline const VkQueue get_graphics_queue() const { return m_graphics_queue; }

        void print_extension_support() const;
        void terminate();

    private:
        static void _populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
        static bool _check_validation_layer_support();
        static VulkanDeviceQueueFamilyIndices _find_rendering_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
        static bool _check_physical_device_required_extensions_support(VkPhysicalDevice physical_device);

        // initialization functions
        void _init_instance();
        void _init_debug_manager();
        void _init_physical_device();
        void _init_logical_device();

    private:
        static bool m_enable_validation_layers;
        static std::vector<const char*> m_validation_layers;
        static std::vector<const char*> m_device_extensions;

        Window* m_window{ nullptr };
        VkInstance m_instance{ nullptr };
        VkPhysicalDevice m_physical_device{ nullptr };
        VkPhysicalDeviceFeatures m_physical_device_features;
        VkDevice m_device{ nullptr };
        VkQueue m_graphics_queue{ nullptr };
        VkQueue m_present_queue{ nullptr };

#if !defined(NDEBUG)
        VkDebugUtilsMessengerEXT m_debug_messenger{ nullptr };
#endif 
    };

}

#endif // __VULKAN_DEVICE_HPP__