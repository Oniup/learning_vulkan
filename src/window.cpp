#include "window.hpp"

namespace vlk {
    Window::Window(int width, int height, std::string title) : m_width(width), m_height(height), m_title(title) {
        _init_window();
    }

    Window::~Window() {
        glfwDestroyWindow(m_internal_window);
        glfwTerminate();
    }

    void Window::_init_window() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not create a context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_internal_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    }
}