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

vec3 SampleLambert(vec3 albedo, vec3 N, inout vec3 L, inout float pdf)
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

void SampleDistantLight(in Light light, in vec3 scatterPos, inout LightSample lightSample)
{
	lightSample.direction = normalize(light.position - vec3(0,0,0));
	lightSample.normal = normalize(scatterPos - light.position);
	lightSample.emission = light.emission; //light.emission*float(ubo.numofLights)
	lightSample.distance = INFINITY;
	lightSample.pdf = 1.0;
}

//Shape sample
void SampleRectLight(in Light light, in vec3 scatterPos, inout LightSample lightSample, float numLights)
{
	float r1 = Random();
	float r2 = Random();
	//point on rect surface
	vec3 lightSurfacePos = light.position + light.u * r1 + light.v * r2;
	lightSample.direction =lightSurfacePos - scatterPos;
	lightSample.distance = length(lightSample.direction);
	float distanceSquare = lightSample.distance * lightSample.distance;
	lightSample.direction /= lightSample.distance;
	lightSample.normal = normalize(cross(light.u, light.v));
	lightSample.emission = light.emission*numLights ; //light.emission * float(ubo.numOfLights);
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

void SampleOneLight(in Light light, in vec3 scatterPos, inout LightSample lightSample,float numLights)
{
	int type = int(light.type);

	if (type == RECT_LIGHT)
		SampleRectLight(light, scatterPos, lightSample,numLights);
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
	float m = clamp(1.0 - u, 0.0, 1.0);
	float m2 = m * m;
	return m2 * m2 * m;
}

//GTR: Generalized-Trowbridge-Reitz,used to estimate specular
float GTR1(float NdotH, float a)
{
	if (a >= 1) return 1 / PI;
	float a2 = a * a;
	float t = 1 + (a2 - 1) * NdotH * NdotH;
	return (a2 - 1) / (PI * log(a2) * t);
}

vec3 SampleGTR1(float rgh, float r1, float r2)
{
	float a = max(0.001, rgh);
	float a2 = a * a;

	float phi = r1 * TWO_PI;

	float cosTheta = sqrt((1.0 - pow(a2, 1.0 - r1)) / (1.0 - a2));
	float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
	float sinPhi = sin(phi);
	float cosPhi = cos(phi);

	return vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
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

vec3 ImportanceSampleGTR2(float rgh, float r1, float r2)
{
	float a = max(0.001, rgh);

	float phi = r1 * TWO_PI;

	float cosTheta = sqrt((1.0 - r2) / (1.0 + (a * a - 1.0) * r2));
	float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
	float sinPhi = sin(phi);
	float cosPhi = cos(phi);

	return vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}

float GTR2Aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
	float a = HdotX / ax;
	float b = HdotY / ay;
	float c = a * a + b * b + NdotH * NdotH;
	return 1.0 / (PI * ax * ay * c * c);
	//return 1 / (PI * ax * ay * sqr(sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH));
}

vec3 ImportanceSampleGTR2_Aniso(float ax, float ay, float r1, float r2)
{
	float phi = r1 * TWO_PI;

	float sinPhi = ay * sin(phi);
	float cosPhi = ax * cos(phi);
	float tanTheta = sqrt(r2 / (1 - r2));

	return vec3(tanTheta * cosPhi, tanTheta * sinPhi, 1.0);
}


float SmithGAniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
	float a = VdotX * ax;
	float b = VdotY * ay;
	float c = NdotV;
	return 1 / (c + sqrt(a * a + b * b + c * c));

	//return 1 / (NdotV + sqrt(sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV)));
}


// Samples a microfacet normal for the GGX distribution using VNDF method.
// Source: "Sampling the GGX Distribution of Visible Normals" by Heitz

vec3 SampleGGXVNDF(vec3 Ve, vec2 alpha2D, vec2 u)
{
	vec3 Vh = normalize(vec3(alpha2D.x * Ve.x, alpha2D.y * Ve.y, Ve.z));
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	vec3 T1 = lensq > 0.0f ? vec3(-Vh.y, Vh.x, 0.0f) * inversesqrt(lensq) : vec3(1, 0, 0);
	vec3 T2 = cross(Vh, T1);

	float r = sqrt(u.x);
	float phi = 2.0 * PI * u.y;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

	vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

	return normalize(vec3(alpha2D.x * Nh.x, alpha2D.y * Nh.y, max(0.0, Nh.z)));
}

float DielectricFresnel(float cosThetaI, float eta)
{
	float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);

	// Total internal reflection
	if (sinThetaTSq > 1.0)
		return 1.0;

	float cosThetaT = sqrt(max(1.0 - sinThetaTSq, 0.0));

	float rs = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);
	float rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);

	return 0.5f * (rs * rs + rp * rp);
}

#endif