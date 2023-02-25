#include "application.hpp"

#include <iostream>

namespace vlk {

    Application::Application() 
        : m_window(Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Learning Vulkan")), m_pipeline(VulkanPipeline("shaders/bin/simple_shader.vert.spv", "shaders/bin/simple_shader.frag.spv")),
          m_device(VulkanDevice(&m_window)) {
    }

    Application::~Application() {
        m_device.terminate();
    }

    void Application::run() {
        while (!m_window.should_close()) {
            glfwPollEvents();
        }
    }

}