#include "definitions.glsl"

vec3 multiplyMV(mat4 mat, vec4 v)
{
    vec4 res = mat * v;
    return vec3(res.xyz);
}

float TriangleIntersectionTest(Triangle triangle, Ray r, inout Intersection intersectionPoint)
{
    vec3 baryPos;
    bool hasIntersect = RayTriangleIntersectionCheck(r.origin, r.direction, triangle.v0, triangle.v1, triangle.v2, baryPos);
    if (!hasIntersect) return -1.f;

    intersectionPoint = (1.f - baryPos.x - baryPos.y) * triangle.v0 + baryPos.x * triangle.v1 + baryPos.y * triangle.v2;

    return glm::length(r.origin - intersectionPoint);
}

//Based on glm::intersectTriangle, based on barypos check
bool RayTriangleIntersectionCheck(vec3 ray_origin,vec3 ray_direction, vec3 triangle_v0, vec3 triangle_v1, inout vec3 baryPosition)
{
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 p = cross(ray_direction, e2);

    //parallel, return false
    float a = dot(e1, p);
    if (a < EPSILON)
        return false;

    float f = (1.0f) / a;
    vec3 s = ray_origin - v0;
    baryPosition.x = f * dot(s, p);
    if (baryPosition.x < 0.0f)
        return false;
    if (baryPosition.x >1.0f)
        return false;

    vec3 q = cross(s, e1);
    baryPosition.y = f * dot(dir, q);
    if (baryPosition.y < 0.0f)
        return false;
    if (baryPosition.y + baryPosition.x > 1.0f)
        return false;

    baryPosition.z = f * dot(e2, q);

    return baryPosition.z >= 0.0f;
}



vec3 Reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
}

