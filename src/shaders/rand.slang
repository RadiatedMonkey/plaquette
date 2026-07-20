implementing lqft;

/// Implements xoshiro256+
struct XoshiroRng {
    uint64_t mState[4];

    /// Initialises the random number generator state using the given seed for the given thread ID.
    /// The thread ID ensures that each thread gets a different seed.
    __init(uint3 dispatchThreadID, uint64_t baseSeed) {
        uint64_t threadID = dispatchThreadID.x;
        uint64_t state = threadID ^ baseSeed;

        mState[0] = splitmix64(state);
        mState[1] = splitmix64(state);
        mState[2] = splitmix64(state);
        mState[3] = splitmix64(state);
    }

    /// Returns the next state of the random number generator.
    [mutating]
    uint64_t nextState() {
        uint64_t result = mState[0] + mState[3];
        uint64_t t = mState[1] << 17;

        mState[2] ^= mState[0];
        mState[3] ^= mState[1];
        mState[1] ^= mState[2];
        mState[0] ^= mState[3];

        mState[2] ^= t;
        mState[3] = rotl(mState[3], 45);

        return result;
    }

    /// Returns the next double. This is done by calling `nextState` and then converting
    /// it to a double using the topmost 53 bits of the state.
    [mutating]
    double nextDouble() {
        uint64_t state = nextState();

        // Take upper 53 bits and divide into range [0, 1).
        return double(state >> 11) * double(1.0 / 9007199254740992.0); // 1 / 2^53
    }

    /// Returns the next float. This is done by calling `nextState` and then converting
    /// it into a float using the topmost 24 bits of the state.
    [mutating]
    float nextFloat() {
        uint64_t state = nextState();

        // Then extract the top 24 bits and divide by 2^24 to map it to [0,1).
        return float(state >> 40) * (1.0f / 16777216.0f); // 1 / 2^24
    }
};

/// splitmix64 generates a random seed from the given state.
uint64_t splitmix64(inout uint64_t state) {
    state += 0x9e3779b97f4a7c15L;

    uint64_t z = state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9L;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebL;
    return z ^ (z >> 31);
}

/// Bitwise rotate left
uint64_t rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}