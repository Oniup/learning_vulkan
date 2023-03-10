cmake_minimum_required (VERSION 3.2)

project ("LearningVulkan")

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set (
    APPLICATION_SOURCES

    src/main.cpp
    src/application.cpp
    src/vulkan_pipeline.cpp
    src/window.cpp
    src/vulkan_device.cpp
)

set (
    APPLICATION_HEADERS

    src/application.hpp
    src/vulkan_pipeline.hpp
    src/window.hpp
    src/vulkan_device.hpp
)

add_executable (${CMAKE_PROJECT_NAME} ${APPLICATION_SOURCES} ${APPLICATION_HEADERS})

find_package (Vulkan)
if (!Vulkan_FOUND)
    message("Need to install Vulkan SDK")
endif ()

add_subdirectory (thirdparty/glfw)

target_include_directories (
    ${CMAKE_PROJECT_NAME}

    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm

    PUBLIC ${GLFW_INCLUDE_DIR}
    PUBLIC ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries (
    ${CMAKE_PROJECT_NAME}

    PUBLIC glfw
    PUBLIC ${Vulkan_LIBRARIES}
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_custom_target(
        shaders

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/bin
        ${CMAKE_BINARY_DIR}/Debug/shaders/bin
    )

    add_dependencies(${CMAKE_PROJECT_NAME} shaders)
endif ()
