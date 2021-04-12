typedef union {
    struct { Vec2 xy; float _z; };
    struct { float x, y, z; };
    float nums[3];
} Vec3;

INLINE Vec3 vec3(float x, float y, float z) {
    return (Vec3){ .nums = { x, y, z } };
}

INLINE Vec3 vec3_f(float f) {
    return vec3(f, f, f);
}

INLINE Vec3 x3(void) {
    return vec3(1.0, 0.0, 0.0);
}

INLINE Vec3 y3(void) {
    return vec3(0.0, 1.0, 0.0);
}

INLINE Vec3 z3(void) {
    return vec3(0.0, 0.0, 1.0);
}

INLINE Vec3 add3(Vec3 a, Vec3 b) {
    return vec3(a.x + b.x,
                a.y + b.y,
                a.z + b.z);
}

INLINE Vec3 sub3(Vec3 a, Vec3 b) {
    return vec3(a.x - b.x,
                a.y - b.y,
                a.z - b.z);
}

INLINE Vec3 div3(Vec3 a, Vec3 b) {
    return vec3(a.x / b.x,
                a.y / b.y,
                a.z / b.z);
}

INLINE Vec3 div3_f(Vec3 a, float f) {
    return vec3(a.x / f,
                a.y / f,
                a.z / f);
}

INLINE Vec3 mul3(Vec3 a, Vec3 b) {
    return vec3(a.x * b.x,
                a.y * b.y,
                a.z * b.z);
}

INLINE Vec3 mul3_f(Vec3 a, float f) {
    return vec3(a.x * f,
                a.y * f,
                a.z * f);
}

INLINE float dot3(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

INLINE Vec3 lerp3(Vec3 a, Vec3 b, float t) {
    return add3(mul3_f(a, 1.0f - t), mul3_f(b, t));
}

INLINE Vec3 quad_bez3(Vec3 a, Vec3 b, Vec3 c, float t) {
    return lerp3(lerp3(a, b, t), lerp3(b, c, t), t);
}

INLINE float mag3(Vec3 a) {
    return sqrtf(dot3(a, a));
}

INLINE float magmag3(Vec3 a) {
    return dot3(a, a);
}

INLINE Vec3 norm3(Vec3 a) {
    return div3_f(a, mag3(a));
}

INLINE bool eq3(Vec3 a, Vec3 b) {
    return a.x == b.x &&
           a.y == b.y &&
           a.z == b.z;
}

/* source: https://math.stackexchange.com/a/44691 */
INLINE Vec3 rand3(void) {
    float theta = randf() * PI_f * 2.0f,
              z = 1.0f - randf() * 2.0f,
             cz = sqrtf(1.0f - powf(z, 2.0f));

    return vec3(cz * cosf(theta),
                cz * sinf(theta),
                z               );
}

INLINE Vec3 cross3(Vec3 a, Vec3 b) {
    return vec3((a.y * b.z) - (a.z * b.y),
                (a.z * b.x) - (a.x * b.z),
                (a.x * b.y) - (a.y * b.x));
}
