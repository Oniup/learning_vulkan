#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include "window.hpp"
#include "vulkan_pipeline.hpp"
#include "vulkan_device.hpp"

namespace vlk {

    class Application {
    public:
        static constexpr int WINDOW_WIDTH = 800;
        static constexpr int WINDOW_HEIGHT = 600;

        Application();
        ~Application();

        void run();

    private:
        Window m_window;
        VulkanDevice* m_device{ nullptr };
        VulkanPipeline* m_pipeline{ nullptr };
    };

}

#endif // __APPLICATION_HPP__