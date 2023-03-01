#include "application.hpp"

#include <iostream>

namespace vlk {

    Application::Application() 
        : m_window(Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Learning Vulkan")), 
          m_device(new VulkanDevice(&m_window)), m_pipeline(new VulkanPipeline(m_device)) {}

    Application::~Application() {
        delete m_pipeline;
        delete m_device;
    }

    void Application::run() {
        while (!m_window.should_close()) {
            glfwPollEvents();
        }
    }

}