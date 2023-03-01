#ifndef __VULKAN_PIPELINE_HPP__
#define __VULKAN_PIPELINE_HPP__

#include "vulkan_device.hpp"

#include <string_view>
#include <vector>

namespace vlk {

    class VulkanPipeline {
    public:
        VulkanPipeline(VulkanDevice* device);
        ~VulkanPipeline();

        std::vector<const char*> read_shader_source(std::string_view shader_path);
        VkShaderModule create_shader_module(const std::vector<const char*>& shader_source);

    private:
        VulkanDevice* m_device{ nullptr };
    };

}

#endif // __VULKAN_PIPELINE_HPP__