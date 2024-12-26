#include "definitions.glsl"

vec3 TriangleIntersection(in vec3 ray_origin, in vec3 ray_direction, Triangle triangle, inout vec3 normal)
{
    vec3 a = triangle.v0 - triangle.v1;
    vec3 b = triangle.v2 - triangle.v0;
    vec3 p = triangle.v0 - ray_origin;
    normal = cross(b, a);

    //If this value is close to zero, it means the ray is parallel to the plane and there is no intersection.
    float det = dot(ray_direction, normal);

    if (det > -EPSILON && det < EPSILON)
    {
        return vec3(0, 0, 0); //in this case no intersection
    }
    float idet = 1.0f / det;

    //Using barycentric coordinates, any point on the triangle can be expressed as a convex combination of the triangle's vertices
    vec3 q = cross(p, ray_direction);

    float u = dot(q, b) * idet;
    float v = dot(q, a) * idet;
    float t = dot(normal, p) * idet;

    return vec3(t, u, v);
}

vec3 Reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
}
