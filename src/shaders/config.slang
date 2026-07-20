implementing lqft;

struct Lattice<F: IFloat> {
    uint4 dimensions;
    uint totalSites;
    F spacing;
    F *buffer;
};

struct PushConstants<F: IFloat> {
    uint64_t baseSeed;

    double *output;

    Lattice<F> lattice;
};

[vk::push_constant]
PushConstants<float> params;