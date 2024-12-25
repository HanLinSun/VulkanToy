#version 450
#include "random.glsl"
#include "definitions.glsl"
#include "intersections.glsl"

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba8) uniform writeonly image2D targetTexture;

layout(binding = 1) uniform UniformBufferObject {
    RayTraceUniformBuffer ubo;
}

layout(std430, binding = 2) readonly buffer TriangleBufferObject
{
    Triangle[] triangles;
}

layout(std430, binding = 3) readonly buffer MaterialBufferObject
{
    Material[] materials;
}

layout(std430, binding = 4) readonly buffer SpheresBufferObject {
    Sphere[] spheres;
};

layout(std430, binding = 5) readonly buffer TextureBufferObject
{
    image2D[] materialTextures;
}

layout(std430, binding = 6) readonly buffer LightBufferObject
{
    Light[] lights;
}
vec3 RayAt(Ray r, double t)
{
    return r.origin + r.direction * float(t);
}

vec3 TriangleIntersection(in vec3 ray_origin, in vec3 ray_direction, Triangle triangle, inout vec3 normal)
{
    vec3 a = triangle.v0 - triangle.v1;
    vec3 b = triangle.v2 - triangle.v0;
    vec3 p = triangle.v0 - ray_origin;
    vec3 normal = cross(b, a);

    //If this value is close to zero, it means the ray is parallel to the plane and there is no intersection.
    flaot det = dot(ray_direction, normal)

        if (det > -EPSILON && det < EPSILON)
        {
            return vec3(0, 0, 0); //in this case no intersection
        }

    float idet = 1.0 / det;

    //Using barycentric coordinates, any point on the triangle can be expressed as a convex combination of the triangle's vertices
    vec3 q = cross(p, ray_direction);

    float u = dot(q, b) * idet;
    float v = dot(q, a) * idet;
    float t = dot(normal, p) * idet;

    return vec3(t, u, v);
}

bool HitTriangle(int triangle_index, float tMin, float tMax, Ray r, inout Intersection intersection)
{
    triangle t = triangles[triangle_index];
    vec3 n = vec3(0, 0, 0);
    vec3 hit = TriangleIntersection(r.origin, r.direction, t, n);
    if (!(hit.y < 0.0 || hit.y>1.0 || hit.z < 0.0 || (hit.y + hit.z)>1.0))
    {
        intersecton.position = r.origin + hit.x * r.direction;
        intersection.normal = normalize(n);
        intersection.backFaceFlag = dot(r.direction, intersection.normal) > 0 ? 1 : 0;
        intersection.normal *= 1 - 2 * intersection.backFaceFlag;
        intersection.position += intersection.normal * 0.0001;
        intersection.t = hit.x;
        intersection.materialIndex = t.materialIdx;
        return hit.x > tMin && hit.x < tMax;
    }
    return false;
}

float HitSphere(int sphere_index, float tMin, float tmax, Ray r, inout Intersection intersection)
{
    vec3 center = spheres[sphere_index].s.xyz;
    float radius = spheres[sphere_index].s.w;

    vec3 oc = center - r.origin;
    float a = dot(r.direction, r.direction);
    float half_b = dot(r.direction, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = half_b * half_b - a * c;

    if (discriminant < 0)
    {
        return -1.0;
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
    intersection.position = RayAt(r, intersection.t);
    intersection.normal = (1 - 2 * intersection.backFaceFlag) * ((intersection.position - center) / radius);
    return true;
}


vec3 TraceRay(Ray r)
{
    double t = HitSphere(vec3(0, 0, -1), 0.3, r);
    if (t > 0.0)
    {
        vec3 sphereNormal = normalize(RayAt(r, t) - vec3(0, 0, -1));
        return 0.5 * vec3(sphereNormal.x + 1, sphereNormal.y + 1, sphereNormal.z + 1);
    }
    vec3 unit_direction = normalize(r.direction);
    float a = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0);
}


void main()
{
    vec2 image_size = vec2(imageSize(targetTexture));
    float theta = ubo.cameraFOV * PI / 180.f;

    float h = tan(theta / 2);
    float viewport_height = 2.0 * h;
    float viewport_width = image_size.x / image_size.y * viewport_height;
    float focal_length = ubo.focalDistance;

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 horizontal = vec3(viewport_width, 0, 0);
    vec3 vertical = vec3(0, -viewport_height, 0);

    vec3 lower_left_corner = ubo.cameraPos - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
    vec2 uv = (gl_GlobalInvocationID.xy) / image_size.xy;

    Ray r;
    r.origin = ubo.cameraPos;
    r.direction = lower_left_corner + uv.x * horizontal + uv.y * vertical - r.origin;

    //Ray Generation


    vec3 pixelColor = TraceRay(r);


    imageStore(targetTexture, ivec2(gl_GlobalInvocationID.xy), vec4(pixelColor, 0.0));

}
