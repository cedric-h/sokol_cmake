typedef union {
    struct { float x, y; };
    struct { float u, v; };
    float nums[2];
} Vec2;

INLINE Vec2 vec2(float x, float y) {
    return (Vec2){ x, y };
}

INLINE Vec2 vec2_f(float f) {
    return vec2(f, f);
}

INLINE Vec2 vec2_rot(float rot) {
    return vec2(cosf(rot), sinf(rot));
}

INLINE float rot_vec2(Vec2 rot) {
    return atan2f(rot.y, rot.x);
}

INLINE Vec2 add2(Vec2 a, Vec2 b) {
    return vec2(a.x + b.x,
                a.y + b.y);
}

INLINE Vec2 add2_f(Vec2 a, float f) {
    return vec2(a.x + f,
                a.y + f);
}

INLINE Vec2 sub2(Vec2 a, Vec2 b) {
    return vec2(a.x - b.x,
                a.y - b.y);
}

INLINE Vec2 sub2_f(Vec2 a, float f) {
    return vec2(a.x - f,
                a.y - f);
}

INLINE Vec2 div2(Vec2 a, Vec2 b) {
    return vec2(a.x / b.x,
                a.y / b.y);
}

INLINE Vec2 div2_f(Vec2 a, float f) {
    return vec2(a.x / f,
                a.y / f);
}

INLINE Vec2 mul2(Vec2 a, Vec2 b) {
    return vec2(a.x * b.x,
                a.y * b.y);
}

INLINE Vec2 mul2_f(Vec2 a, float f) {
    return vec2(a.x * f,
                a.y * f);
}

INLINE float dot2(Vec2 a, Vec2 b) {
    return a.x*b.x + a.y*b.y;
}

INLINE Vec2 lerp2(Vec2 a, Vec2 b, float t) {
    return add2(mul2_f(a, 1.0f - t), mul2_f(b, t));
}

INLINE float mag2(Vec2 a) {
    return sqrtf(dot2(a, a));
}

INLINE float magmag2(Vec2 a) {
    return dot2(a, a);
}

INLINE Vec2 norm2(Vec2 a) {
    return div2_f(a, mag2(a));
}

INLINE bool eq2(Vec2 a, Vec2 b) {
    return a.x == b.x &&
           a.y == b.y;
}

INLINE Vec2 rand2(void) {
    float theta = randf() * PI_f * 2.0f;
    return vec2(cosf(theta),
                sinf(theta));
}

INLINE Vec2 perp2(Vec2 a) {
    return vec2(a.y, -a.x);
}

/* see https://github.com/SRombauts/SimplexNoise/blob/master/src/SimplexNoise.cpp#L45 */
INLINE int32_t fastfloor(float fp) {
    int32_t i = (int32_t) (fp);
    return (fp < i) ? (i - 1) : (i);
}

static uint8_t simplex_gradients[256] = {0};
/* note: you probably want to seed_rand(x, y, z, w) before calling me */
static void seed_simplex() {
    int i = 0;
    while (i < 256) {
        uint32_t r = rand32();
        simplex_gradients[i++] = (uint8_t) (r << 0 );
        simplex_gradients[i++] = (uint8_t) (r << 8 );
        simplex_gradients[i++] = (uint8_t) (r << 16);
        simplex_gradients[i++] = (uint8_t) (r << 24);
    }
}


INLINE uint8_t hash(int32_t i) {
    return simplex_gradients[(uint8_t) i];
}

static float grad(int32_t hash, float x, float y) {
    const int32_t h = hash & 0x3F;  // Convert low 3 bits of hash code
    const float u = h < 4 ? x : y;  // into 8 simple gradient directions,
    const float v = h < 4 ? y : x;
    // and compute the dot product with (x,y).
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

INLINE float simplex2(Vec2 pnt) {
    float x = pnt.x, y = pnt.y, n0, n1, n2;   // Noise contributions from the three corners

    // Skewing/Unskewing factors for 2D
    static const float F2 = 0.366025403f;  // F2 = (sqrt(3) - 1) / 2
    static const float G2 = 0.211324865f;  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)

    // Skew the input space to determine which simplex cell we're in
    const float s = (x + y) * F2;  // Hairy factor for 2D
    const float xs = x + s;
    const float ys = y + s;
    const int32_t i = fastfloor(xs);
    const int32_t j = fastfloor(ys);

    // Unskew the cell origin back to (x,y) space
    const float t = (float) (i + j) * G2;
    const float X0 = i - t;
    const float Y0 = j - t;
    const float x0 = x - X0;  // The x,y distances from the cell origin
    const float y0 = y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int32_t i1, j1;  // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0 > y0) {   // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        i1 = 1;
        j1 = 0;
    } else {   // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        i1 = 0;
        j1 = 1;
    }

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6

    const float x1 = x0 - i1 + G2;            // Offsets for middle corner in (x,y) unskewed coords
    const float y1 = y0 - j1 + G2;
    const float x2 = x0 - 1.0f + 2.0f * G2;   // Offsets for last corner in (x,y) unskewed coords
    const float y2 = y0 - 1.0f + 2.0f * G2;

    // Work out the hashed gradient indices of the three simplex corners
    const int gi0 = hash(i + hash(j));
    const int gi1 = hash(i + i1 + hash(j + j1));
    const int gi2 = hash(i + 1 + hash(j + 1));

    // Calculate the contribution from the first corner
    float t0 = 0.5f - x0*x0 - y0*y0;
    if (t0 < 0.0f) {
        n0 = 0.0f;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    // Calculate the contribution from the second corner
    float t1 = 0.5f - x1*x1 - y1*y1;
    if (t1 < 0.0f) {
        n1 = 0.0f;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    // Calculate the contribution from the third corner
    float t2 = 0.5f - x2*x2 - y2*y2;
    if (t2 < 0.0f) {
        n2 = 0.0f;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 45.23065f * (n0 + n1 + n2);
}

INLINE float line_dist2(Vec2 start, Vec2 end, Vec2 pt) {
    Vec2 line_dir = sub2(end, start);
    Vec2 perp_dir = perp2(line_dir);
    Vec2 to_start_dir = sub2(start, pt);
    return fabsf(dot2(norm2(perp_dir), to_start_dir));
}

