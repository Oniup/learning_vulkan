#include "vulkan_pipeline.hpp"

#include <iostream>
#include <stdexcept>

namespace vlk {

    VulkanPipeline::VulkanPipeline(std::string_view vert_file, std::string_view frag_file) {
        create_graphics_pipeline(vert_file, frag_file);
    }

    std::vector<char> VulkanPipeline::read_file(std::string_view file_path) {
        std::FILE* file = std::fopen(file_path.data(), "rb");

        if (file == nullptr) {
            std::cout << "failed to open file: " << file_path.data() << "\n";
            std::exit(-1);
        }

        std::fseek(file, 0, SEEK_END);
        size_t length = static_cast<size_t>(std::ftell(file));
        std::fseek(file, 0, SEEK_SET);

        std::vector<char> buffer = std::vector<char>(length);
        std::fread(buffer.data(), buffer.size(), sizeof(char), file);
        std::fclose(file);

        return buffer;
    }

    void VulkanPipeline::create_graphics_pipeline(std::string_view vert_file, std::string_view frag_file) {
        auto vertex = read_file(vert_file);
        auto fragment = read_file(frag_file);

        std::cout << "Vertex Shader Binary Size: " << vertex.size() << "\n";
        std::cout << "Fragment Shader Binary Size: " << fragment.size() << "\n";
    }

}