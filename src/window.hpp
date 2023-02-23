#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <string>

namespace vlk {

    class Window {
    public:
        Window(int width, int height, std::string title);
        ~Window();

        inline bool should_close() { return glfwWindowShouldClose(m_internal_window); }
    private:
        void _init_window();

        Window(const Window& other) = delete;
        Window(Window&& other) = delete;
        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) = delete;

        const int m_width;
        const int m_height;
        std::string m_title;
        GLFWwindow* m_internal_window = nullptr;
    };

}

#endif