#include "vulkan_pipeline.hpp"

#include <iostream>

namespace vlk {

    std::vector<VkDynamicState> VulkanPipeline::m_dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

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

        _init_pipeline_layout();

        vkDestroyShaderModule(m_device->get_device(), vertex, nullptr);
        vkDestroyShaderModule(m_device->get_device(), fragment, nullptr);
    }

    VulkanPipeline::~VulkanPipeline() {
        vkDestroyPipelineLayout(m_device->get_device(), m_layout, nullptr);
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

    void VulkanPipeline::_init_pipeline_layout() {
        // TODO: come back here later to properly configure these settings
        VkPipelineDynamicStateCreateInfo selected_dynamic_states{};
        selected_dynamic_states.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        selected_dynamic_states.dynamicStateCount = static_cast<uint32_t>(m_dynamic_states.size());
        selected_dynamic_states.pDynamicStates = m_dynamic_states.data();

        VkPipelineVertexInputStateCreateInfo vertex_input{};
        vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input.vertexBindingDescriptionCount = 0;
        vertex_input.pVertexBindingDescriptions = nullptr;
        vertex_input.vertexAttributeDescriptionCount = 0;
        vertex_input.pVertexAttributeDescriptions = nullptr;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = m_device->get_swapchain_extent().width;
        viewport.height = m_device->get_swapchain_extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_device->get_swapchain_extent();

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;         // pushes everything further than the far culling point will be brought to that point, needs gpu feature enabled
        rasterizer.rasterizerDiscardEnable = VK_FALSE;  // geometry never passes through the rasterizer basically disables the output to the framebuffer
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = VK_TRUE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_state{};
        color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state.logicOpEnable = VK_FALSE;
        color_blend_state.logicOp = VK_LOGIC_OP_COPY;
        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = &color_blend_attachment_state;

        VkPipelineLayoutCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        create_info.setLayoutCount = 0;
        create_info.pSetLayouts = nullptr;
        create_info.pushConstantRangeCount = 0;
        create_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_device->get_device(), &create_info, nullptr, &m_layout) != VK_SUCCESS) {
            std::cout << "failed to create pipeline layout\n";
            std::exit(-1);
        }

        std::cout << "successfully created vulkan pipeline layout\n";
    }

}