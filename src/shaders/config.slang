implementing lqft;

struct PushConstants {
    uint64_t baseSeed;
    uint totalSites;
};

[vk::push_constant]
PushConstants params;