module lqft;

__include rand;
__include config;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(
    uint3 dispatchThreadID: SV_DispatchThreadID
) {
    if (dispatchThreadID.x > params.totalSites) {
        return; // Thread is outside of lattice
    }

    XoshiroRng rng = XoshiroRng(dispatchThreadID, params.baseSeed);
    double d = rng.nextDouble();

    printf("%f", d);
}