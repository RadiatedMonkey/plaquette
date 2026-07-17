mkdir build\bin\shaders
slangc src/compute.slang -profile glsl_450 -target spirv -o build/bin/shaders/compute.spv