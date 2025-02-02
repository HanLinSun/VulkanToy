#include "definitions.glsl"

vec3 TriangleIntersectionTest(in vec3 ray_origin, in vec3 ray_direction, Triangle triangle, inout vec3 normal)
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

void AreaLightIntersectionTest(in Ray r, inout float closest, in Light light, inout LightSample lightSample)
{
    float dist = HitRectPlane(r, light);
    if (dist < 0.0) dist = INFINITY;
    if (dist < closest)
    {
        closest = dist;
        vec3 normal = normalize(cross(light.u, light.v));
        float cosTheta = abs(-r.direction, normal);
        float pdf = t * t / (light.area * cosTheta);
        lightSample.pdf = pdf;
        lightSample.emission = light.emission;
        lightSample.normal = normal;
    }
}

void SphereLightIntersectionTest(in Ray r, inout float closest, in Light light, inout LightSample lightSample)
{
    float dist = HitSphereLight(r, light);
    if (dist < 0.0) dist = INFINITY;

    if (dist < closest)
    {
        closest = dist;

        vec3 surfacePos = r.origin + r.direction *closest;
        vec3 normal = normalize(surfacePos - light.position);
        float pdf = (dist * dist) / light.area;

        lightSample.emission = light.emission;
        lightSample.pdf = pdf;
        lightSample.normal = normal;
    }
}

vec3 Reflect(vec3 v, vec3 n) {
    return v - 2 * dot(v, n) * n;
}
vec3 multiplyMV(mat4 mat, vec4 v)
{
    vec4 res =mat * v;
    return vec3(res.xyz);
}

bool HitTriangle(Primitive prim, Ray r, float tMin, float tMax, inout Intersection intersection)
{
    vec3 ray_origin = multiplyMV(prim.inverseTransform, vec4(r.origin, 1.0f));
    vec3 ray_direction = multiplyMV(prim.inverseTransform, vec4(r.direction, 0.0f));

    Triangle t = prim.triangle;
    vec3 n = vec3(0, 0, 0);
    vec3 hit = TriangleIntersectionTest(ray_origin, ray_direction, t, n);
    if (!(hit.y < 0.0 || hit.y>1.0 || hit.z < 0.0 || (hit.y + hit.z)>1.0))
    {
        intersection.position = ray_origin + hit.x * ray_direction;
        intersection.normal = normalize(n);
        intersection.backFaceFlag = dot(ray_direction, intersection.normal) > 0 ? 1 : 0;
        intersection.normal *= 1 - 2 * intersection.backFaceFlag;
        intersection.position += intersection.normal * 0.0001;

        //back to world space
        intersection.position = multiplyMV(prim.transform, vec4(intersection.position, 1.0));
        intersection.normal = multiplyMV(prim.inverseTranspose, vec4(intersection.normal, 0.0));

        intersection.t = hit.x;
        intersection.material_ID = prim.materialIdx;
        return hit.x > tMin && hit.x < tMax;
    }
    return false;
}

bool HitBoundingBox(Ray r, vec3 boxMin, vec3 boxMax)
{
    vec3 invDir = 1.0 / r.direction;
    //Multply is much faster than divide
    vec3 f = (boxMax - r.origin) * invDir;
    vec3 n = (boxMin - r.origin) * invDir;
    // world space intersection test

    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);

    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    float t0 = max(tmin.x, max(tmin.y, tmin.z));

    return t1 >= t0;
}

bool HitSphere(Primitive prim, Ray r, float tMin, float tMax, inout Intersection intersection)
{
    vec3 ray_origin = multiplyMV(prim.inverseTransform, vec4(r.origin, 1.0f));
    vec3 ray_direction = multiplyMV(prim.inverseTransform, vec4(r.direction, 0.0f));
    vec3 center = prim.sphere.s.xyz;
    float radius = prim.sphere.s.w;
    vec3 oc = center - ray_origin;
    float a = dot(ray_direction, ray_direction);
    float half_b = dot(ray_direction, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = half_b * half_b - a * c;

    if (discriminant < 0)
    {
        return false;
    }
    float sqrt_d = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    float root = (half_b - sqrt_d) / a;
    intersection.backFaceFlag = 0;
    if (root < tMin || tMax < root)
    {
        root = (half_b + sqrt_d) / a;
        intersection.backFaceFlag = 1;
        if (root <= tMin || tMax <= root)
            return false;
    }

    intersection.t = root;

    intersection.position = ray_origin + intersection.t * ray_direction;
    intersection.normal = (1 - 2 * intersection.backFaceFlag) * ((intersection.position - center) / radius);

    //back to world space
    intersection.position = multiplyMV(prim.transform, vec4(intersection.position, 1.0));
    intersection.normal = multiplyMV(prim.inverseTranspose, vec4(intersection.normal, 0.0));
    return true;
}

bool HitPrimitive(Primitive prim, float tMin, float tMax, Ray r, inout Intersection intersection)
{
    if (prim.type == 0)
    {
        return HitTriangle(prim, r, tMin, tMax, intersection);
    }
    else if (prim.type == 1)
    {
        return HitSphere(prim, r, tMin, tMax, intersection);
    }
}

float HitRectPlane(in Ray r, in Light light)
{
    vec3 u = light.u;
    vec3 v = light.v;

    vec3 normal = normalize(cross(u, v));
    vec4 plane = vec4(normal, dot(normal, light.position));
    u* = 1.0 / u * u;
    v *= 1.0 / v * v;

    vec3 n = vec3(plane);

    float dt = dot(r.direction, n);
    float t = (plane.w - dot(n, r.origin)) / dt;

    if (t > EPSILON)
    {
        vec3 p = r.origin + r.direction * t;
        vec3 vi = p - pos;
        float a1 = dot(u, vi);
        if (a1 >= 0.0 && a1 <= 1.0)
        {
            float a2 = dot(v, vi);
            if (a2 >= 0.0 && a2 <= 1.0)
                return t;
        }
    }

    //Not hit rect plane
    return INFINITY;
}

float HitSphereLight(in Ray r, in Light light)
{
    vec3 dir = light.position - r.origin;
    float b = dot(dir, r.direction);
    float det = b * b - dot(dir, dir) + light.radius * light.radius;

    if (det < 0.0) return INFINITY;

    det = sqrt(det);

    float t1 = b - det;
    if (t1 >EPSILON) return t1;

    float t2 = b + det;
    if (t2 > EPSILON) return t2;

    return INFINITY;
}