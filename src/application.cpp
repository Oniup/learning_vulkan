#include "application.hpp"

namespace vlk {

    void Application::run() {
        while (!m_window.should_close()) {
            glfwPollEvents();
        }
    }

}