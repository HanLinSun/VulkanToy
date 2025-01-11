#ifndef SHARED_SAMPLE
#define SHARED_SAMPLE

#include "random.glsl"
#include "definitions.glsl"
#define PIOver2  1.57079632679489661923f
#define PIOver4  0.78539816339744830961f
//Based on pbr-book.org Sampling Algorithms
vec3 SampleUniformHemisphere(float r1, float r2)
{
	vec3 dir;
	float r = sqrt(max(0.0, 1.0 - r1 * r1));
	float phi = 2 * PI * r2;
	dir.x = r * cos(phi);
	dir.y = r * sin(phi);
	dir.z = r1;
	return dir;
}

vec3 SampleUniformSphere(float r1, float r2)
{
	float z = 1 - 2 * r1;
	float r = sqrt(1 - z * z);
	float phi = 2 * PI * r2;
	return vec3(r * cos(phi), r * sin(phi), z);
}

float CosineHemispherePDF(float cosTheta) {
	return cosTheta * INV_PI;
}

vec2 SampleUniformDiskConcentric(float r1,float r2)
{
	vec2 u = vec2(r1, r2);
	vec2 uOffset = 2 * u - vec2(1, 1);
	if (uOffset.x == 0 && uOffset.y == 0)
	{
		return vec2(0, 0);
	}
	float theta;
	float r;
	if (abs(uOffset.x) > abs(uOffset.y))
	{
		r = uOffset.x;
		float divide = uOffset.y / uOffset.x;
		theta = PIOver4 * divide;
	}
	else
	{
		r = uOffset.y;
		float divide = uOffset.y / uOffset.x;
	    theta=PIOver2 -PIOver4 *divide;
	}
	return vec2(r*cos(theta), r*sin(theta));
}

vec3 CosineSampleHemisphere(float r1, float r2)
{
	vec2 d = SampleUniformDiskConcentric(r1, r2);
	float z = sqrt(max(0.0, 1 - d.x * d.x - d.y * d.y));
	return vec3(d.x, d.y, z);
}

float UniformHemispherePDF()
{
	return INV_2PI;
}

void Onb(in vec3 N, inout vec3 T, inout vec3 B)
{
	vec3 up = abs(N.z) < 0.9999999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
	T = normalize(cross(up, N));
	B = cross(N, T);
}

vec3 SampleLambert(vec3 albedo,vec3 V, vec3 N, inout vec3 L, inout float pdf)
{
	float r1 = Random();
	float r2 = Random();

	vec3 T, B;
	Onb(N, T, B);

	L = CosineSampleHemisphere(r1, r2);
	L = T * L.x + B * L.y + N * L.z;

	pdf = dot(N, L) * (1.0 / PI);

	return (1.0 / PI) * albedo * dot(N, L);
}
#endif