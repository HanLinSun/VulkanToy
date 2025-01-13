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

//Compute tangent space tangent and bitangent
//Orthonormal Basis
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

void SampleDistantLight(in Light light, in vec3 scatterPos, inout LightSampleRec lightSample)
{
	lightSample.direction = normalize(light.position - vec3(0,0,0));
	lightSample.normal = normalize(scatterPos - light.position);
	lightSample.emission = light.emission; //light.emission*float(ubo.numofLights)
	lightSample.distance = INFINITY;
	lightSample.pdf = 1.0;
}

//Shape sample
void SampleRectLight(in Light light, in vec3 scatterPos, inout LightSample lightSample)
{
	float r1 = Random();
	float r2 = Random();
	//point on rect surface
	vec3 lightSurfacePos = light.position + light.u * r1 + light.v * r2;
	lightSample.direction = normalize(lightSurfacePos - scatterPos);
	lightSample.distance = length(lightSample.direction);
	float distanceSquare = lightSample.distance * lightSample.distance;
	lightSample.normal = normalize(cross(light.u, light.v));
	lightSample.emission = light.emission; //light.emission * float(ubo.numOfLights);
	lightSample.pdf = distanceSquare / (light.area * abs(dot(lightSample.normal, lightSample.direction)));
}

void SampleSphereLight(in Light light, in vec3 scatterPos, inout LightSample lightSample)
{
	float r1 = Random();
	float r2 = Random();
	vec3 sphereCentertoSurface = scatterPos - light.position;
	float distToSphereCenter = length(sphereCentertoSurface);
	vec3 sampledDir;

	// TODO: Fix this. Currently assumes the light will be hit only from the outside
	sphereCentertoSurface /= distToSphereCenter;
	sampledDir = SampleUniformHemisphere(r1, r2);
	vec3 T, B;
	Onb(sphereCentertoSurface, T, B);
	sampledDir = T * sampledDir.x + B * sampledDir.y + sphereCentertoSurface * sampledDir.z;

	vec3 lightSurfacePos = light.position + sampledDir * light.radius;

	lightSample.direction = normalize(lightSurfacePos - scatterPos);
	lightSample.distance = length(lightSample.direction);
	float distSquare = lightSample.distance * lightSample.distance;

//	lightSample.direction /= lightSample.dist;
	lightSample.normal = normalize(lightSurfacePos - light.position);
	lightSample.emission = light.emission; //light.emission *float(numOfLights);
	lightSample.pdf = distSquare / (light.area * 0.5 * abs(dot(lightSample.normal, lightSample.direction)));
}

void SampleOneLight(in Light light, in vec3 scatterPos, inout LightSample lightSample)
{
	int type = int(light.type);

	if (type == RECT_LIGHT)
		SampleRectLight(light, scatterPos, lightSample);
	else if (type == SPHERE_LIGHT)
		SampleSphereLight(light, scatterPos, lightSample);
	else
		SampleDistantLight(light, scatterPos, lightSample);
}

float PowerHeuristic(float a, float b)
{
	float t = a * a;
	return t / (b * b + t);
}

//Disney BRDF sample algorithm
//Reference: [https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf] 
//Reference 2: [https://schuttejoe.github.io/post/disneybsdf/] and related paper help me to implement disney PBR sample

// pow(m,5)
float SchlickFresnel(float u)
{
	float m = clamp(1 - u, 0, 1);
	float m2 = m * m;
	return m2 * m2 * m; // pow(m,5)
}

//GTR: Generalized-Trowbridge-Reitz,used to estimate specular
float GTR1(float NdotH, float a)
{
	if (a >= 1) return 1 / PI;
	float a2 = a * a;
	float t = 1 + (a2 - 1) * NdotH * NdotH;
	return (a2 - 1) / (PI * log(a2) * t);
}

//This is the original version in disney bsdf
float SmithGGX(float NdotV, float alphaG)
{
	float a = alphaG * alphaG;
	float b = NdotV * NdotV;
	return 1 / (NdotV + sqrt(a + b - a * b));
}

float GTR2(float NdotH, float a)
{
	float a2 = a * a;
	float t = 1 + (a2 - 1) * NdotH * NdotH;
	return a2 / (PI * t * t);
}

float SmithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
	return 1 / (NdotV + sqrt(sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV)));
}

vec3 mon2lin(vec3 x)
{
	return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

#endif