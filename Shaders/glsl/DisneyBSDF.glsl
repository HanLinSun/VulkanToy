//Disney PBR BRDF
//CopyRight Hanlin Sun 2025
/**References:
1: [https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf] Disney BRDF implements
2: [https://schuttejoe.github.io/post/disneybsdf/] and related paper help me to implement disney PBR sample
3: [https://blog.selfshadow.com/publications/s2015-shading-course/] SIGGRAPH 2015 Course: Physically Based Shading in Theory and Practice
4. [https://github.com/mmacklin/tinsel/blob/master/src/disney.h] mmacklin's implements on Disney PBR
5. 
**/

#include "sample.glsl"
#include "definitions.glsl"

vec3 DisneyEval(Intersection intersection, vec3 V, vec3 N, vec3 L, out float pdf);

vec3 ToWorld(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return V.x * X + V.y * Y + V.z * Z;
}

vec3 ToLocal(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return V.x * X + V.y * Y + V.z * Z;
}

vec3 Luminance(vec3 baseColor)
{
    return dot(vec3(0.3f, 0.6f, 1.0f), baseColor);
}
//============  Sheen ================
vec3 CalculateTint(vec3 baseColor)
{
    float luminance = Luminance(baseColor);
    return (luminance > 0.0f) ? baseColor * (1.0f / luminance) : vec3(1.0);
}

//L: incoming light. in BRDF, wi
//H: half vector
//V: outcome direction
vec3 EvalSheen(PBRMaterial material, vec3 V, vec3 H,vec3 L)
{
    float LdotH = dot(H, L);
    vec3 tint = CalculateTint(material.baseColor);
    return material.sheen * mix(vec3(1.0f), tint, material.sheenTint) * SchlickWeight(LdotH);
}

vec3 EvalSpecularTint(PBRMaterial material, float eta )
{
    vec3 tint = CalculateTint(material.baseColor);

    float F0 = (1.0 - eta) / (1.0 + eta);
    F0 *= F0;

   return  F0 * mix(vec3(1.0), tint, mat.specularTint);
}

//
float EvalDisneyClearcoat(float clearcoat, float alpha, )
{

}
//================================


