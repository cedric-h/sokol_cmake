typedef union {
    struct {
        union {
            Vec3 xyz;
            struct { float x, y, z; };
        };

        float w;
    };

    Vec4 xyzw;
    float nums[4];
} Quat;

INLINE Quat quat(float x, float y, float z, float w) {
    return (Quat){ .nums = { x, y, z, w } };
}

INLINE Quat vec4Q(Vec4 v) {
    return quat(v.x, v.y, v.z, v.w);
}

INLINE Vec4 quat4(Quat q) {
    return vec4(q.x, q.y, q.z, q.w);
}

INLINE Quat identQ(void) {
    return quat(0.0, 0.0, 0.0, 1.0);
}
INLINE Quat yprQ(float yaw, float pitch, float roll) {
    float y0 = sinf(yaw   * 0.5f), w0 = cosf(yaw   * 0.5f),
          x1 = sinf(pitch * 0.5f), w1 = cosf(pitch * 0.5f),
          z2 = sinf(roll  * 0.5f), w2 = cosf(roll  * 0.5f);

    float x3 = w0 * x1,
          y3 = y0 * w1,
          z3 = -y0 * x1,
          w3 = w0 * w1;

    float x4 = x3 * w2 + y3 * z2,
          y4 = -x3 * z2 + y3 * w2,
          z4 = w3 * z2 + z3 * w2,
          w4 = w3 * w2 - z3 * z2;

    return quat(x4, y4, z4, w4);
}

INLINE Quat axis_angleQ(Vec3 axis, float angle) {
    Vec3 axis_norm = norm3(axis);
    float rot_sin = sinf(angle / 2.0f);

    Quat res;
    res.xyz = mul3_f(axis_norm, rot_sin);
    res.w = cosf(angle / 2.0f);
    return res;
}
