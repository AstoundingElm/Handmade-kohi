echo on

echo "Compiling shaders..."

/home/petermiller/Desktop/1.3.216.0/x86_64/bin/glslc -fshader-stage=vert assets/shaders/Builtin.ObjectShader.vert.glsl -o assets/shaders/Builtin.ObjectShader.vert.spv

/home/petermiller/Desktop/1.3.216.0/x86_64/bin/glslc -fshader-stage=frag assets/shaders/Builtin.ObjectShader.frag.glsl -o assets/shaders/Builtin.ObjectShader.frag.spv

cp -R "assets" "bin" 