typedef union {
    struct { float x, y, z, w; };
    Vec3 xyz;
    Vec2 xy;

    float nums[4];
} Vec4;

INLINE Vec4 vec4(float x, float y, float z, float w) {
    return (Vec4){ .nums = { x, y, z, w } };
}

INLINE Vec4 vec4_f(float f) {
    return vec4(f, f, f, f);
}

INLINE Vec4 x4(void) {
    return vec4(1.0, 0.0, 0.0, 0.0);
}

INLINE Vec4 y4(void) {
    return vec4(0.0, 1.0, 0.0, 0.0);
}

INLINE Vec4 z4(void) {
    return vec4(0.0, 0.0, 1.0, 0.0);
}

INLINE Vec4 w4(void) {
    return vec4(0.0, 0.0, 0.0, 1.0);
}

INLINE Vec4 add4(Vec4 a, Vec4 b) {
    return vec4(a.x + b.x,
                a.y + b.y,
                a.z + b.z,
                a.w + b.w);
}

INLINE Vec4 sub4(Vec4 a, Vec4 b) {
    return vec4(a.x - b.x,
                a.y - b.y,
                a.z - b.z,
                a.w - b.w);
}

INLINE Vec4 div4(Vec4 a, Vec4 b) {
    return vec4(a.x / b.x,
                a.y / b.y,
                a.z / b.z,
                a.w / b.w);
}

INLINE Vec4 div4_f(Vec4 a, float f) {
    return vec4(a.x / f,
                a.y / f,
                a.z / f,
                a.w / f);
}

INLINE Vec4 mul4(Vec4 a, Vec4 b) {
    return vec4(a.x * b.x,
                a.y * b.y,
                a.z * b.z,
                a.w * b.w);
}

INLINE Vec4 mul4_f(Vec4 a, float f) {
    return vec4(a.x * f,
                a.y * f,
                a.z * f,
                a.w * f);
}

INLINE float dot4(Vec4 a, Vec4 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

INLINE Vec4 lerp4(Vec4 a, Vec4 b, float t) {
    return add4(mul4_f(a, 1.0f - t), mul4_f(b, t));
}

INLINE float mag4(Vec4 a) {
    return sqrtf(dot4(a, a));
}

INLINE float magmag4(Vec4 a) {
    return dot4(a, a);
}

INLINE Vec4 norm4(Vec4 a) {
    return div4_f(a, mag4(a));
}

INLINE bool eq4(Vec4 a, Vec4 b) {
    return a.x == b.x &&
           a.y == b.y &&
           a.z == b.z &&
           a.w == b.w;
}
