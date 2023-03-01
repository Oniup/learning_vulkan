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
        ~VulkanDevice();

        inline VkInstance get_instance() { return m_instance; }
        inline VkDevice get_device() { return m_device; }
        inline VkPhysicalDevice get_physical_device() { return m_physical_device; }
        inline VkSwapchainKHR get_swapchain() { return m_swapchain; }
        inline VkQueue get_graphics_queue() { return m_graphics_queue; }
        inline VkQueue get_present_mode_queue() { return m_present_queue; }

        inline const VkInstance get_instance() const { return m_instance; }
        inline const VkDevice get_device() const { return m_device; }
        inline const VkPhysicalDevice get_physical_device() const { return m_physical_device; }
        inline const VkSwapchainKHR get_swapchain() const { return m_swapchain; }
        inline const VkExtent2D& get_swapchain_extent() const { return m_swapchain_extent; }
        inline const VkQueue get_graphics_queue() const { return m_graphics_queue; }
        inline const VkQueue get_present_mode_queue() const { return m_present_queue; }
        inline const VkPhysicalDeviceFeatures& get_physical_device_features() const { return m_physical_device_features; }

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