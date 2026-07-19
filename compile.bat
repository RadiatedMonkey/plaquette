mkdir build\debug\bin\shaders
slangc src/compute.slang -profile glsl_450 -target spirv -o build/debug/bin/shaders/compute.spv