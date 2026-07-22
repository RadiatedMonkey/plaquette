typedef double FScalar;

struct Lattice {
    uint4 dimensions;
    uint totalSites;
    FScalar spacing;
    FScalar *buffer;
};

struct PushConstants {
    uint64_t baseSeed;
    uint dispatched;

    FScalar *output;

    Lattice lattice;
};

[vk::push_constant]
PushConstants params;