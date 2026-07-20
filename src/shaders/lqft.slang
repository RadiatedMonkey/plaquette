module lqft;

__include rand;
__include config;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(
    uint3 dispatchThreadID: SV_DispatchThreadID
) {
    uint rngsPerThread = params.totalSites / 64;

    if (dispatchThreadID.x > params.totalSites) {
        return; // Thread is outside of lattice
    }

    XoshiroRng rng = XoshiroRng(dispatchThreadID, params.baseSeed);
    for (uint i = 0; i < rngsPerThread; i++) {
        double f = double(rng.nextFloat());
        params.output[rngsPerThread * dispatchThreadID.x + i] = f;
    }
}