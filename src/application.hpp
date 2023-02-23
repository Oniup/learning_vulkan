#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include "window.hpp"
#include "vulkan_pipeline.hpp"
#include <string>

namespace vlk {

    class Application {
    public:
        static constexpr int WINDOW_WIDTH = 800;
        static constexpr int WINDOW_HEIGHT = 600;

        void run();

    private:
        Window m_window { Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Learning Vulkan") };
        VulkanPipeline m_pipeline { VulkanPipeline("shaders/bin/simple_shader.vert.spv", "shaders/bin/simple_shader.frag.spv") };
    };

}

#endif // __APPLICATION_HPP__