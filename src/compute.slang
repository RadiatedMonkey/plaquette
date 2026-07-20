struct PushConstants {
    float *buffer0;
    float *buffer1;
    float *result;
};

[vk::push_constant]
PushConstants pushed;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadId: SV_DispatchThreadID) {
    uint index = threadId.x;

    pushed.result[index] = pushed.buffer0[index] + pushed.buffer1[index];
}