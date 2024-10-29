%VK_SDK_PATH%\Bin\glslc.exe test_shader.vert -o test_shader.vert.spv
%VK_SDK_PATH%\Bin\glslc.exe test_shader.frag -o test_shader.frag.spv
%VK_SDK_PATH%\Bin\glslc.exe point_light.vert -o point_light.vert.spv
%VK_SDK_PATH%\Bin\glslc.exe point_light.frag -o point_light.frag.spv
%VK_SDK_PATH%\Bin\glslc.exe pbr.vert -o pbr.vert.spv
%VK_SDK_PATH%\Bin\glslc.exe mat_unlit.frag -o mat_unlit.frag.spv
%VK_SDK_PATH%\Bin\glslc.exe mat_pbr.frag -o mat_pbr.frag.spv
pause