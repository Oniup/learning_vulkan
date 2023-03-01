#include "vulkan_pipeline.hpp"

#include <iostream>

namespace vlk {

    VulkanPipeline::VulkanPipeline(VulkanDevice* device) : m_device(device) {
        std::vector<const char*> vertex_source = read_shader_source("shaders/bin/simple_shader.vert.spv");
        std::vector<const char*> fragment_source = read_shader_source("shaders/bin/simple_shader.frag.spv");

        VkShaderModule vertex = create_shader_module(vertex_source);
        VkShaderModule fragment = create_shader_module(fragment_source);
        VkPipelineShaderStageCreateInfo stage_create_info[] = { {}, {} };

        stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_create_info[0].module = vertex;
        stage_create_info[0].pName = "main";
        stage_create_info[0].pSpecializationInfo = nullptr;

        stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_create_info[1].module = fragment;
        stage_create_info[1].pName = "main";
        stage_create_info[1].pSpecializationInfo = nullptr;

        vkDestroyShaderModule(m_device->get_device(), vertex, nullptr);
        vkDestroyShaderModule(m_device->get_device(), fragment, nullptr);
    }

    VulkanPipeline::~VulkanPipeline() {
    }

    std::vector<const char*> VulkanPipeline::read_shader_source(std::string_view shader_path) {
        std::FILE* file = std::fopen(shader_path.data(), "rb");
        if (file == nullptr) {
            std::cout << "vulkan pipeline failed to get shader source from path: \"" << shader_path << "\"\n";
            std::exit(-1);
        }

        std::fseek(file, 0, SEEK_END);
        std::vector<const char*> source(std::ftell(file));
        std::fseek(file, 0, SEEK_SET);

        std::fread(source.data(), source.size(), 1, file);
        std::fclose(file);

        return source;
    }

    VkShaderModule VulkanPipeline::create_shader_module(const std::vector<const char*>& shader_source) {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = shader_source.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(shader_source.data());

        VkShaderModule shader{};
        if (vkCreateShaderModule(m_device->get_device(), &create_info, nullptr, &shader) != VK_SUCCESS) {
            std::cout << "failed to create shader module\n";
            std::exit(-1);
        }
        return shader;
    }

}