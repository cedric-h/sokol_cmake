typedef union {
    struct { Vec4 x, y, z, w; };
    Vec4 cols[4];
    float nums[4][4];
} Mat4;

INLINE Mat4 mul4x4(Mat4 a, Mat4 b) {
    Mat4 out = {0};
    int8_t k, r, c;
    for (c = 0; c < 4; ++c)
        for (r = 0; r < 4; ++r) {
            out.nums[c][r] = 0.0f;
            for (k = 0; k < 4; ++k)
                out.nums[c][r] += a.nums[k][r] * b.nums[c][k];
        }
    return out;
}

INLINE Vec4 mul4x44(Mat4 m, Vec4 v) {
    Vec4 res;
    for(int x = 0; x < 4; ++x) {
        float sum = 0;
        for(int y = 0; y < 4; ++y)
            sum += m.nums[y][x] * v.nums[y];

        res.nums[x] = sum;
    }
    return res;
}

INLINE Mat4 mat4x4(void) {
    return (Mat4){0};
}

INLINE Mat4 diag4x4(float f) {
    Mat4 res = mat4x4();
    res.nums[0][0] = f;
    res.nums[1][1] = f;
    res.nums[2][2] = f;
    res.nums[3][3] = f;
    return res;
}

INLINE Mat4 ident4x4(void) {
    return diag4x4(1.0f);
}


INLINE Mat4 scale4x4(Vec3 scale) {
    Mat4 res = ident4x4();
    res.nums[0][0] = scale.x;
    res.nums[1][1] = scale.y;
    res.nums[2][2] = scale.z;
    return res;
}

INLINE Mat4 translate4x4(Vec3 pos) {
    Mat4 res = diag4x4(1.0);
    res.nums[3][0] = pos.x;
    res.nums[3][1] = pos.y;
    res.nums[3][2] = pos.z;
    return res;
}

INLINE Mat4 invert4x4(Mat4 a) {
    float s[6], c[6];
    s[0] = a.nums[0][0]*a.nums[1][1] - a.nums[1][0]*a.nums[0][1];
    s[1] = a.nums[0][0]*a.nums[1][2] - a.nums[1][0]*a.nums[0][2];
    s[2] = a.nums[0][0]*a.nums[1][3] - a.nums[1][0]*a.nums[0][3];
    s[3] = a.nums[0][1]*a.nums[1][2] - a.nums[1][1]*a.nums[0][2];
    s[4] = a.nums[0][1]*a.nums[1][3] - a.nums[1][1]*a.nums[0][3];
    s[5] = a.nums[0][2]*a.nums[1][3] - a.nums[1][2]*a.nums[0][3];

    c[0] = a.nums[2][0]*a.nums[3][1] - a.nums[3][0]*a.nums[2][1];
    c[1] = a.nums[2][0]*a.nums[3][2] - a.nums[3][0]*a.nums[2][2];
    c[2] = a.nums[2][0]*a.nums[3][3] - a.nums[3][0]*a.nums[2][3];
    c[3] = a.nums[2][1]*a.nums[3][2] - a.nums[3][1]*a.nums[2][2];
    c[4] = a.nums[2][1]*a.nums[3][3] - a.nums[3][1]*a.nums[2][3];
    c[5] = a.nums[2][2]*a.nums[3][3] - a.nums[3][2]*a.nums[2][3];
    
    /* Assumes it is invertible */
    float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
    
    Mat4 res;
    res.nums[0][0] = ( a.nums[1][1] * c[5] - a.nums[1][2] * c[4] + a.nums[1][3] * c[3]) * idet;
    res.nums[0][1] = (-a.nums[0][1] * c[5] + a.nums[0][2] * c[4] - a.nums[0][3] * c[3]) * idet;
    res.nums[0][2] = ( a.nums[3][1] * s[5] - a.nums[3][2] * s[4] + a.nums[3][3] * s[3]) * idet;
    res.nums[0][3] = (-a.nums[2][1] * s[5] + a.nums[2][2] * s[4] - a.nums[2][3] * s[3]) * idet;

    res.nums[1][0] = (-a.nums[1][0] * c[5] + a.nums[1][2] * c[2] - a.nums[1][3] * c[1]) * idet;
    res.nums[1][1] = ( a.nums[0][0] * c[5] - a.nums[0][2] * c[2] + a.nums[0][3] * c[1]) * idet;
    res.nums[1][2] = (-a.nums[3][0] * s[5] + a.nums[3][2] * s[2] - a.nums[3][3] * s[1]) * idet;
    res.nums[1][3] = ( a.nums[2][0] * s[5] - a.nums[2][2] * s[2] + a.nums[2][3] * s[1]) * idet;

    res.nums[2][0] = ( a.nums[1][0] * c[4] - a.nums[1][1] * c[2] + a.nums[1][3] * c[0]) * idet;
    res.nums[2][1] = (-a.nums[0][0] * c[4] + a.nums[0][1] * c[2] - a.nums[0][3] * c[0]) * idet;
    res.nums[2][2] = ( a.nums[3][0] * s[4] - a.nums[3][1] * s[2] + a.nums[3][3] * s[0]) * idet;
    res.nums[2][3] = (-a.nums[2][0] * s[4] + a.nums[2][1] * s[2] - a.nums[2][3] * s[0]) * idet;

    res.nums[3][0] = (-a.nums[1][0] * c[3] + a.nums[1][1] * c[1] - a.nums[1][2] * c[0]) * idet;
    res.nums[3][1] = ( a.nums[0][0] * c[3] - a.nums[0][1] * c[1] + a.nums[0][2] * c[0]) * idet;
    res.nums[3][2] = (-a.nums[3][0] * s[3] + a.nums[3][1] * s[1] - a.nums[3][2] * s[0]) * idet;
    res.nums[3][3] = ( a.nums[2][0] * s[3] - a.nums[2][1] * s[1] + a.nums[2][2] * s[0]) * idet;
    return res;
}

INLINE Mat4 transpose4x4(Mat4 a) {
    Mat4 res;
    for(int c = 0; c < 4; ++c)
        for(int r = 0; r < 4; ++r)
            res.nums[r][c] = a.nums[c][r];
    return res;
}

INLINE Mat4 look_at4x4(Vec3 eye, Vec3 center, Vec3 up) {
    Mat4 res;

    Vec3 f = norm3(sub3(center, eye));
    Vec3 s = norm3(cross3(f, up));
    Vec3 u = cross3(s, f);

    res.nums[0][0] = s.x;
    res.nums[0][1] = u.x;
    res.nums[0][2] = -f.x;
    res.nums[0][3] = 0.0f;

    res.nums[1][0] = s.y;
    res.nums[1][1] = u.y;
    res.nums[1][2] = -f.y;
    res.nums[1][3] = 0.0f;

    res.nums[2][0] = s.z;
    res.nums[2][1] = u.z;
    res.nums[2][2] = -f.z;
    res.nums[2][3] = 0.0f;

    res.nums[3][0] = -dot3(s, eye);
    res.nums[3][1] = -dot3(u, eye);
    res.nums[3][2] = dot3(f, eye);
    res.nums[3][3] = 1.0f;

    return res;
}

INLINE Mat4 perspective4x4(float fov, float aspect, float near, float far) {
    float cotangent = 1.0f / tanf(fov * (PI_f / 360.0f));

    Mat4 res = mat4x4();
    res.nums[0][0] = cotangent / aspect;
    res.nums[1][1] = cotangent;
    res.nums[2][3] = -1.0f;
    res.nums[2][2] = (near + far) / (near - far);
    res.nums[3][2] = (2.0f * near * far) / (near - far);
    res.nums[3][3] = 0.0f;
    return res;
}

INLINE Mat4 ortho4x4(float left, float right, float bottom, float top, float near, float far) {
    Mat4 res = ident4x4();

    res.nums[0][0] = 2.0f / (right - left);
    res.nums[1][1] = 2.0f / (top - bottom);
    res.nums[2][2] = 2.0f / (near - far);
    res.nums[3][3] = 1.0f;

    res.nums[3][0] = (left + right) / (left - right);
    res.nums[3][1] = (bottom + top) / (bottom - top);
    res.nums[3][2] = (far + near) / (near - far);

    return res;
}

INLINE Mat4 quat4x4(Quat q) {
    Mat4 res = diag4x4(1.0);
    Quat norm = vec4Q(norm4(q.xyzw));

    float xx = norm.x * norm.x,
          yy = norm.y * norm.y,
          zz = norm.z * norm.z,
          xy = norm.x * norm.y,
          xz = norm.x * norm.z,
          yz = norm.y * norm.z,
          wx = norm.w * norm.x,
          wy = norm.w * norm.y,
          wz = norm.w * norm.z;

    res.nums[0][0] = 1.0f - 2.0f * (yy + zz);
    res.nums[0][1] = 2.0f * (xy + wz);
    res.nums[0][2] = 2.0f * (xz - wy);
    res.nums[0][3] = 0.0f;

    res.nums[1][0] = 2.0f * (xy - wz);
    res.nums[1][1] = 1.0f - 2.0f * (xx + zz);
    res.nums[1][2] = 2.0f * (yz + wx);
    res.nums[1][3] = 0.0f;

    res.nums[2][0] = 2.0f * (xz + wy);
    res.nums[2][1] = 2.0f * (yz - wx);
    res.nums[2][2] = 1.0f - 2.0f * (xx + yy);
    res.nums[2][3] = 0.0f;

    res.nums[3][0] = 0.0f;
    res.nums[3][1] = 0.0f;
    res.nums[3][2] = 0.0f;
    res.nums[3][3] = 1.0f;
    return res;
}

INLINE Mat4 ypr4x4(float yaw, float pitch, float roll) {
    return quat4x4(yprQ(yaw, pitch, roll));
}

INLINE Mat4 axis_angle4x4(Vec3 axis, float angle) {
    return quat4x4(axis_angleQ(axis, angle));
}
