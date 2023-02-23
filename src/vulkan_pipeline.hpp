#ifndef __VULKAN_PIPELINE_HPP__
#define __VULKAN_PIPELINE_HPP__

#include <string_view>
#include <vector>

namespace vlk {

    class VulkanPipeline {
    public:
        VulkanPipeline(std::string_view vert_file, std::string_view frag_file);
        ~VulkanPipeline() = default;

    private:
        static std::vector<char> read_file(std::string_view file_path);

        void create_graphics_pipeline(std::string_view vert_file, std::string_view frag_file);
    };

}

#endif // __VULKAN_PIPELINE_HPP__