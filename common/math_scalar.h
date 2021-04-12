/* actually handy because windows's CRT doesn't have pow(int, int) */
#define square(x) (x*x)

#define clamp(min, val, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))

INLINE float lerp(float a, float b, float t) {
    return (1.0f - t) * a + t * b;
}
INLINE float inv_lerp(float a, float b, float v) {
    return fminf(1.0f, fmaxf(0.0f, (v - a) / (b - a)));
}
INLINE float smoothstep_ex(float edge0, float edge1, float x) {
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f); 
    return x * x * (3.0f - 2.0f * x);
}
INLINE float smoothstep(float x) {
    return smoothstep_ex(0.0f, 1.0f, x);
}

INLINE uint32_t rotl(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

static uint32_t _math_rand_seed[4];

/* source: http://prng.di.unimi.it/xoshiro128plus.c
   NOTE: The state must be seeded so that it is not everywhere zero. */
static uint32_t rand32(void) {
    const uint32_t result = _math_rand_seed[0] + _math_rand_seed[3],
                        t = _math_rand_seed[1] << 9;

    _math_rand_seed[2] ^= _math_rand_seed[0];
    _math_rand_seed[3] ^= _math_rand_seed[1];
    _math_rand_seed[1] ^= _math_rand_seed[2];
    _math_rand_seed[0] ^= _math_rand_seed[3];

    _math_rand_seed[2] ^= t;

    _math_rand_seed[3] = rotl(_math_rand_seed[3], 11);

    return result;
}

INLINE void seed_rand(uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3) {
    _math_rand_seed[0] = s0;
    _math_rand_seed[1] = s1;
    _math_rand_seed[2] = s2;
    _math_rand_seed[3] = s3;
}

INLINE float randf(void) {
    return (rand32() >> 8) * 0x1.0p-24f;
}

