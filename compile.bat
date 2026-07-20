mkdir build\debug\bin\shaders
slangc src/shaders/lqft.slang -profile glsl_450 -target spirv -entry main -o build/debug/bin/shaders/compute.spv