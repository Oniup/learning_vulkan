#ifndef __VULKAN_DEVICE_HPP__
#define __VULKAN_DEVICE_HPP__

#include "window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace vlk {

    struct QueueFamilyIndices {
        float priority{ 1.0f };
        std::optional<uint32_t> graphics_family{};
        std::optional<uint32_t> present_family{};

        static QueueFamilyIndices query(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

        inline bool supports_rendering() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};

        static SwapchainSupportDetails query(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

        bool is_supported() const;
        VkSurfaceFormatKHR choose_surface_format();
        VkPresentModeKHR choose_present_mode();
        VkExtent2D choose_swapchain_extent(Window* window);
    };

    class VulkanDevice {
    public:
        static std::vector<const char*> get_required_extensions();

        VulkanDevice(Window* window);
        ~VulkanDevice() = default;

        inline VkInstance get_vulkan_instance() { return m_instance; }
        inline const VkInstance get_vulkan_instance() const { return m_instance; }
        inline VkQueue get_graphics_queue() { return m_graphics_queue; }
        inline const VkQueue get_graphics_queue() const { return m_graphics_queue; }

        void print_extension_support() const;
        void terminate();

    private:
        static void _populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
        static bool _check_validation_layer_support();
        static bool _check_physical_device_required_extensions_support(VkPhysicalDevice physical_device);

        // initialization functions
        void _init_instance();
        void _init_debug_manager();
        void _init_physical_device();
        void _init_logical_device();
        void _init_swapchain();
        void _init_swapchain_images();

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

        VkSwapchainKHR m_swapchain{ nullptr };
        VkSurfaceFormatKHR m_swapchain_surface_format{};
        VkPresentModeKHR m_swapchain_present_mode{};
        VkExtent2D m_swapchain_extent{};
        std::vector<VkImage> m_swapchain_images{};
        std::vector<VkImageView> m_swapchain_image_views{};

#if !defined(NDEBUG)
        VkDebugUtilsMessengerEXT m_debug_messenger{ nullptr };
#endif 
    };

}

#endif // __VULKAN_DEVICE_HPP__