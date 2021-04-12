INLINE Vec3 project_plane_vec3(Vec3 n, Vec3 bd) {
    return sub3(bd, mul3_f(n, dot3(bd, n)));
}

typedef struct {
    Vec3 origin, vector;
} Ray;

INLINE Vec3 ray_hit_plane(Ray ray, Ray plane) {
    float d = dot3(sub3(plane.origin, ray.origin), plane.vector)
               / dot3(ray.vector, plane.vector);
    return add3(ray.origin, mul3_f(ray.vector, d));
}
