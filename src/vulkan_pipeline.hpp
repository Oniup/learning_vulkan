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

        inline static std::vector<VkDynamicState> get_dynamic_states() { return m_dynamic_states; }
        inline VkPipelineLayout get_layout() { return m_layout; }
        inline const VkPipelineLayout get_layout() const { return m_layout; }

        std::vector<const char*> read_shader_source(std::string_view shader_path);
        VkShaderModule create_shader_module(const std::vector<const char*>& shader_source);

    private:
        void _init_pipeline_layout();

        static std::vector<VkDynamicState> m_dynamic_states;

        VulkanDevice* m_device{ nullptr };
        VkPipelineLayout m_layout{ nullptr };
    };

}

#endif // __VULKAN_PIPELINE_HPP__