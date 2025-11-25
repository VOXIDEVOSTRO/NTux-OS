#include "random.h"
#include <stdint.h>
#include <stddef.h>

static inline uint64_t rdtsc64(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}


static uint64_t splitmix64_state;

static uint64_t splitmix64_next(void) {
    uint64_t z = (splitmix64_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}


static uint64_t s0 = 0;
static uint64_t s1 = 0;
static int rng_initialized = 0;

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t random_u64(void) {
    if (!rng_initialized) {
        random_init(rdtsc64());
    }

    const uint64_t result = s0 + s1;

    const uint64_t t = s1 << 17;

    s1 ^= s0;
    s0 = rotl(s0, 49) ^ s1 ^ t; 
    s1 = rotl(s1, 28);

    return result;
}

uint32_t random_u32(void) {
    return (uint32_t)random_u64();
}


void random_init(uint64_t seed) {
    if (seed == 0) {
        seed = rdtsc64();
        if (seed == 0) seed = 0xDEADBEEFCAFEBABEULL; 
    }
    splitmix64_state = seed;
    s0 = splitmix64_next();
    s1 = splitmix64_next();
    if (s0 == 0 && s1 == 0) {
        s0 = 0x0123456789ABCDEFULL;
        s1 = 0xFEDCBA9876543210ULL;
    }

    rng_initialized = 1;
}

bool random_is_initialized(void) {
    return rng_initialized != 0;
}

static uint64_t random_bounded_u64(uint64_t bound) {
    if (bound == 0) return 0;
    uint64_t mask;
    if ((bound & (bound - 1)) == 0) {
        return random_u64() & (bound - 1);
    }
    uint64_t x;
    uint64_t threshold = (uint64_t)(-bound) % bound; 
    do {
        x = random_u64();
    } while (x < threshold);
    return x % bound;
}

uint32_t random_range_u32(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    uint64_t range = (uint64_t)max - (uint64_t)min + 1ULL;
    uint64_t r = random_bounded_u64(range);
    return (uint32_t)(min + r);
}

uint64_t random_range_u64(uint64_t min, uint64_t max) {
    if (min >= max) return min;
    uint64_t range = max - min + 1ULL;
    uint64_t r = random_bounded_u64(range);
    return min + r;
}

void random_bytes(void *buf, size_t len) {
    uint8_t *p = (uint8_t*)buf;
    while (len >= 8) {
        uint64_t v = random_u64();
        p[0] = (v >> 0) & 0xFF;
        p[1] = (v >> 8) & 0xFF;
        p[2] = (v >> 16) & 0xFF;
        p[3] = (v >> 24) & 0xFF;
        p[4] = (v >> 32) & 0xFF;
        p[5] = (v >> 40) & 0xFF;
        p[6] = (v >> 48) & 0xFF;
        p[7] = (v >> 56) & 0xFF;
        p += 8;
        len -= 8;
    }
    if (len > 0) {
        uint64_t v = random_u64();
        for (size_t i = 0; i < len; i++) {
            p[i] = (v >> (i*8)) & 0xFF;
        }
    }
}