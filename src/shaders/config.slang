implementing lqft;

struct PushConstants {
    uint64_t baseSeed;
    uint totalSites;

    float* output;
};

[vk::push_constant]
PushConstants params;