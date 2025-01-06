#include "definitions.glsl"

vec3 multiplyMV(mat4 mat, vec4 v)
{
    vec4 res = mat * v;
    return vec3(res.xyz);
}

//Based on glm::intersectTriangle, based on barypos check
bool RayTriangleIntersectionCheck(vec3 ray_origin,vec3 ray_direction, vec3 v0, vec3 v1, vec3 v2, inout vec3 baryPosition)
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
    baryPosition.y = f * dot(ray_direction, q);
    if (baryPosition.y < 0.0f)
        return false;
    if (baryPosition.y + baryPosition.x > 1.0f)
        return false;

    baryPosition.z = f * dot(e2, q);

    return baryPosition.z >= 0.0f;
}

bool HitBoundingBox(Ray r, vec3 boxMin, vec3 boxMax)
{
    // world space intersection test
    vec3 tMin = (boxMin - r.origin) / r.direction;
    vec3 tMax = (boxMax - r.origin) / r.direction;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    if (tNear >= tFar) return false;
    else return true;
}


float TriangleIntersectionTest(Triangle triangle, Ray r, inout Intersection intersection)
{
    vec3 baryPos=vec3(0,0,0);
    bool hasIntersect = RayTriangleIntersectionCheck(r.origin, r.direction, triangle.v0, triangle.v1, triangle.v2, baryPos);
    if (!hasIntersect) return -1.f;

    intersection.position = (1.f - baryPos.x - baryPos.y) * triangle.v0 + baryPos.x * triangle.v1 + baryPos.y * triangle.v2;

    return length(r.origin - intersection.position);
}


vec3 Reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
}

