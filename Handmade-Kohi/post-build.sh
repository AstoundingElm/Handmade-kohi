echo on

echo "Compiling shaders..."

$VULKAN_SDK/bin/glslc.exe -fshader-stage=vert assets/shaders/Builtin.ObjectShader.vert.glsl -o assets/shaders/Builtin.ObjectShader.vert.spv