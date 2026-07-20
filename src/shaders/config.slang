implementing lqft;

struct PushConstants {
    uint64_t baseSeed;
    uint totalSites;

    double* output;
};

[vk::push_constant]
PushConstants params;