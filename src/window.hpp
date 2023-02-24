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

        inline int get_width() const { return m_width; }
        inline int get_height() const { return m_height; }
        inline const std::string& get_title() const { return m_title; }
        inline GLFWwindow* get_internal_window() { return m_internal_window; }
        inline const GLFWwindow* get_internal_window() const { return m_internal_window; }

        void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);
        bool should_close();
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