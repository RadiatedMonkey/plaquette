__include config;

[shader("compute")]
[numthreads(256, 1, 1)]
void evaluateMean(
    uint3 dispatchThreadID: SV_DispatchThreadID,
    uint3 groupThreadID: SV_GroupThreadID,
    uint3 groupID: SV_GroupID
) {
    uint threadID = dispatchThreadID.x;
    if (threadID >= params.lattice.totalSites) {
        return;
    }

    uint stride = params.dispatched;

    FScalar sum = F(0.0f);
    for (uint i = 0; i < params.lattice.totalSites; i += stride) {
        sum = sum + F(params.lattice.buffer[i]);
    }
}

groupshared FScalar sdata[64];

FScalar aggregateAvg(
    uint threadID, FScalar *values, uint valueCount
) {
    uint stride = params.dispatched;
    FScalar sum = 0.0f;

    for (uint i = threadID; i < valueCount; i += stride) {
        if (i > valueCount) break;

        sum += values[i];
    }

    // Reduce to average per wave.

    FScalar waveAvg = WaveActiveSum(sum) / WaveGetLaneCount();

    uint waveIndex = threadID / WaveGetLaneCount();
    if (WaveIsFirstLane()) {
        sdata[waveIndex] = waveAvg;
    }
    GroupMemoryBarrierWithGroupSync();

    // Reduce wave averages to total average. Only wave 0 gets into action here.
    if (waveIndex == 0) {
        
    }
}

FScalar evaluateMean(
    uint threadID, uint groupThreadID
) {
    if (threadID >= params.lattice.totalSites) {
        return 0.0;
    }

    uint stride = params.dispatched;
    FScalar sum = 0.0f;

    for (uint i = threadID; i < params.lattice.totalSites; i += stride) {
        sum += params.lattice.buffer[i];
    }

    // Then compute average of entire wave
    FScalar waveAvg = WaveActiveSum(sum) / WaveGetLaneCount();

    // Index of this wave in the group.
    uint waveIndex = groupThreadID / WaveGetLaneCount();

    if (WaveIsFirstLane()) {
        waveAvgs[waveIndex] = waveAvg;
    }

    GroupMemoryBarrierWithGroupSync();


}