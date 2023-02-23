:: NOTE: this will not work if your Vulkan SDK is located in a different directory
::       please chane the path to glslc to the corret location for your system

C:\VulkanSDK\1.3.239.0\Bin\glslc.exe shaders\src\simple_shader.vert -o shaders\bin\simple_shader.vert.spv
C:\VulkanSDK\1.3.239.0\Bin\glslc.exe shaders\src\simple_shader.frag -o shaders\bin\simple_shader.frag.spv

PAUSE