//Disney PBR BSDF
//CopyRight Hanlin Sun 2025
/**References:
1: [https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf] Disney BRDF implements
2: [https://schuttejoe.github.io/post/disneybsdf/] and related paper help me to implement disney PBR sample
3: [https://blog.selfshadow.com/publications/s2015-shading-course/] SIGGRAPH 2015 Course: Physically Based Shading in Theory and Practice
4. [https://github.com/mmacklin/tinsel/blob/master/src/disney.h] mmacklin's implements on Disney PBR
5. [https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf] Disney BRDF paper
6. [http://simon-kallweit.me/rendercompo2015/report/] Simon Kallweit's project report
7. [https://github.com/mitsuba-renderer/mitsuba3] mitsuba3
8. [https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf] Microfacet Models for Refraction through Rough Surfaces
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
//N: surface normal
//L: incoming light. in BRDF, wi
//H: half vector
//V: outcome direction
vec3 EvalSheen(in PBRMaterial material, vec3 V, vec3 H,vec3 L)
{
    float LdotH = dot(H, L);
    vec3 tint = CalculateTint(material.baseColor);
    return material.sheen * mix(vec3(1.0f), tint, material.sheenTint) * SchlickWeight(LdotH);
}

vec3 EvalSpecularTint(in PBRMaterial material, float eta )
{
    vec3 tint = CalculateTint(material.baseColor);

    float F0 = (1.0 - eta) / (1.0 + eta);
    F0 *= F0;

   return  F0 * mix(vec3(1.0), tint, mat.specularTint);
}

//Based on Disney bsdf implements
//Clearcoat
vec3 EvalDisneyClearcoat(in PBRMaterial material, vec3 V, vec3 N, vec3 L ,vec3 H, out float pdf )
{
    pdf = 0.0;
    if (dot(N, L) <= 0.0)
        return vec3(0.0);

    float LdotH = dot(L, H);
    float NdotH = dot(N, H);
    float Dr = GTR1(NDotH, mix(0.1,0.001, material.clearcoatGloss));
    float FH = SchlickFresnel(LdotH);
    float F = mix(0.04, 1.0, FH);
    float G = SmithGGX(dot(N, L), 0.25) * SmithGGX(dot(N, V), 0.25);

    //For importance sampling, it is convenient to choose pdfh = D(��h) * cos ��h given that it is already normalized.
    pdf = D * NdotH / (4.0 * dot(V, H)); 

    return vec3(0.25 * material.clearcoat * F * D * G);
}

vec3 EvalDisneyDiffuse(in PBRMaterial material, in vec3 Csheen, vec3 V, vec3 N, vec3 L, vec3 H, inout float pdf)
{
    pdf = 0.0;
    if (dot(N, L) <= 0.0)
        return vec3(0.0);

    float NdotL = dot(N, L);
    float NdotV = dot(N,V);
    float LdotH = dot(L,H);
    //Diffuse PDF: cos(theta) /PI 
    pdf = dot(N, L) * (1.0 / PI);
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float FH = SchlickFresnel(LdotH);
    float Fd90 = 0.5 + 2 * LdotH * LdotH * material.roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Fake subsurface
    float Fss90 = dot(L, H) * dot(L, H) * material.roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (dot(N, L) + dot(N, V)) - 0.5) + 0.5);

    vec3 Fsheen = FH * material.sheen * Csheen;
    return ((1.0 / PI) * mix(Fd, ss, material.subsurface) * material.albedo.xyz + Fsheen) * (1.0 - material.metallic);
}

//Abstract of disney specular(it is divided into reflection and refraction)
vec3 EvalMicrofacetReflection(in PBRMaterial mat, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf)
{
    float aspect = sqrt(1 - mat.anisotropic * 0.9);
    float ax = max(.001, sqr(mat.roughness) / aspect);
    float ay = max(.001, sqr(mat.roughness) * aspect);

    pdf = 0.0;
    if (L.z <= 0.0)
        return vec3(0.0);

    //Disney specular
    float D = GTR2Aniso(H.z, H.x, H.y, ax, ay);
    float G1 = SmithGAniso(abs(V.z), V.x, V.y, ax, ay);
    float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, ax, ay);

    pdf = G1 * D / (4.0 * V.z); 
    return F * D * G2 / (4.0 * L.z * V.z);
}

//Microfacet Models for Refraction through Rough Surfaces from cornell
vec3 EvalMicrofacetRefraction(in PBRMaterial mat, float eta, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf)
{
    pdf = 0.0;
    if (L.z >= 0.0)
        return vec3(0.0);

    float LDotH = dot(L, H);
    float VDotH = dot(V, H);

    float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);
    float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);
    float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);
    float denom = LDotH + VDotH * eta;
    denom *= denom;
    float eta2 = eta * eta;
    float jacobian = abs(LDotH) / denom;

    pdf = G1 * max(0.0, VDotH) * D * jacobian / V.z;
    return pow(mat.baseColor, vec3(0.5)) * (1.0 - F) * D * G2 * abs(VDotH) * jacobian * eta2 / abs(L.z * V.z);
}



