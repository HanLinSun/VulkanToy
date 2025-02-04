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
9. [https://boksajak.github.io/files/CrashCourseBRDF.pdf] Sample Disney BRDF
**/
 
#include "sample.glsl"
#include "definitions.glsl"


vec3 ToWorld(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return V.x * X + V.y * Y + V.z * Z;
}

vec3 ToLocal(vec3 X, vec3 Y, vec3 Z, vec3 V)
{
    return V.x * X + V.y * Y + V.z * Z;
}

float Luminance(vec3 c)
{
    return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
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
vec3 EvalSheen(in PBRMaterial material, vec3 H,vec3 L)
{
    float LdotH = dot(H, L);
    vec3 tint = CalculateTint(material.baseColor);
    return material.sheen * mix(vec3(1.0f), tint, material.sheenTint) * SchlickFresnel(LdotH);
}

vec3 EvalSpecularTint(in PBRMaterial material, float eta )
{
    vec3 tint = CalculateTint(material.baseColor);

    float F0 = (1.0 - eta) / (1.0 + eta);
    F0 *= F0;

   return  F0 * mix(vec3(1.0), tint, material.specularTint);
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
    float D = GTR1(NdotH, mix(0.1,0.001, material.clearcoatGloss));
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
    return ((1.0 / PI) * mix(Fd, ss, material.subsurface) * material.baseColor.xyz + Fsheen) * (1.0 - material.metallic);
}

//Abstract of disney specular(it is divided into reflection and refraction)
vec3 EvalMicrofacetReflection(in PBRMaterial mat, vec3 V, vec3 L, vec3 H, vec3 F, out float pdf)
{
    float aspect = sqrt(1 - mat.anisotropic * 0.9);

    float ax = mat.ax;
    float ay = mat.ay;

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
vec3 EvalDisney(Intersection intersection, PBRMaterial material, vec3 V, vec3 N, vec3 L, out float pdf)
{
    pdf = 0.0;
    vec3 f = vec3(0.0);

    vec3 T, B;
    Onb(N, T, B);
   
    // (NDotL = L.z; NDotV = V.z; NDotH = H.z)
    V = ToLocal(T, B, N, V);
    L = ToLocal(T, B, N, L);

    vec3 H;
    if (L.z > 0.0)
        H = normalize(L + V);
    else
        H = normalize(L + V * intersection.eta);
    

    if (H.z < 0.0)
        H = -H;

    //Tint colors
    vec3 Csheen, Cspec0;
    float F0;
    Csheen = EvalSheen(material, H, L);
    Cspec0 = EvalSpecularTint(material, intersection.eta);

    //Model Weights
    float dielectricWt = (1.0 - material.metallic) * (1.0 - material.specTrans);
    float metalWt = material.metallic;
    float glassWt = (1.0 - material.metallic) * material.specTrans;

    // Lobe probabilities
    float schlickWt = SchlickFresnel(V.z);

    float diffusePr = dielectricWt * Luminance(material.baseColor);
    float dielectricPr = dielectricWt * Luminance(mix(Cspec0, vec3(1.0), schlickWt));
    float metalPr = metalWt * Luminance(mix(material.baseColor, vec3(1.0), schlickWt));
    float glassPr = glassWt;
    float clearCtPr = 0.25 * material.clearcoat;

    //Normalize 
    float invTotalWt = 1.0 / (diffusePr + dielectricPr + metalPr + glassPr + clearCtPr);

    diffusePr *= invTotalWt;
    dielectricPr *= invTotalWt;
    metalPr *= invTotalWt;
    glassPr *= invTotalWt;
    clearCtPr *= invTotalWt;

    bool reflect = L.z * V.z > 0;

    float tmpPdf = 0.0;
    float VDotH = abs(dot(V, H));

    // Diffuse

    if (diffusePr > 0.0 && reflect)
    {
        f += EvalDisneyDiffuse(material, Csheen, V, N, L, H, tmpPdf) * dielectricWt;
        pdf += tmpPdf * diffusePr;
    }

    //Dielectric Reflection
    if (dielectricPr > 0.0 && reflect)
    {
        float F = (DielectricFresnel(VDotH, 1.0 / material.ior) - F0) / (1.0 - F0);
        f += EvalMicrofacetReflection(material, V, L, H, mix(Cspec0, vec3(1.0), F), tmpPdf) * dielectricWt;
        pdf += tmpPdf * dielectricPr;
    }
    //Metallic
    if (metalPr > 0.0 && reflect)
    {
        vec3 F = mix(material.baseColor, vec3(1.0), SchlickFresnel(VDotH));
        f += EvalMicrofacetReflection(material, V, L, H, F, tmpPdf) * metalWt;
        pdf += tmpPdf * metalPr;
    }

    //Glass Specular
    if (glassPr > 0.0)
    {
        // Dielectric fresnel (achromatic)
        float F = DielectricFresnel(VDotH, intersection.eta);

        if (reflect)
        {
            f += EvalMicrofacetReflection(material, V, L, H, vec3(F), tmpPdf) * glassWt;
            pdf += tmpPdf * glassPr * F;
        }
        else
        {
            f += EvalMicrofacetRefraction(material, intersection.eta, V, L, H, vec3(F), tmpPdf) * glassWt;
            pdf += tmpPdf * glassPr * (1.0 - F);
        }
    }

    // Clearcoat
    if (clearCtPr > 0.0 && reflect)
    {
        f += EvalDisneyClearcoat(material, V, N, L, H, tmpPdf) * 0.25 * material.clearcoat;
        pdf += tmpPdf * clearCtPr;
    }
    return f * abs(L.z);
}



//Implements from https://schuttejoe.github.io/post/disneybsdf/
//I have tried to search why this is correct but I failed, including papers or textbooks
//Extending Disney BRDF to BSDF does not mention how to sample different lobes
vec3 SampleDisney(Intersection intersection, PBRMaterial material ,vec3 V, vec3 N, out vec3 L, out float pdf)
{
    pdf = 0.0f;
    float r1 = Random();
    float r2 = Random();

    return N;

    vec3 T, B;
    Onb(N, T, B);

    //Transform to local space
    V = ToLocal(T, B, N, V);


    vec3 Csheen, Cspec0;
    float F0;
    vec3 tint = CalculateTint(material.baseColor);
    Csheen = material.sheen * mix(vec3(1.0f), tint, material.sheenTint);
    Cspec0 = EvalSpecularTint(material, intersection.eta);

    //AnisotropicParams ax,ay
    vec2 a_xy = vec2(material.ax,material.ay);

    //it ensures that the sampling process is proportional to the likelihood of each reflection type contributing to the final color.
    // Importance Sampling
    // Model weights
    float dielectricBRDF = (1.0 - material.metallic) * (1.0 - material.specTrans);
    float metalBRDF = material.metallic;
    float glassBRDF = (1.0 - material.metallic) * material.specTrans;

    //Lobe probabilities
    float schlickWt = SchlickFresnel(V.z);
    float diffusePr = dielectricBRDF * Luminance(material.baseColor);
    float dielectricPr = dielectricBRDF * Luminance(mix(Cspec0, vec3(1.0), schlickWt));
    float metalPr = metalBRDF * Luminance(mix(material.baseColor, vec3(1.0), schlickWt));
    float glassPr = glassBRDF;
    float clearCtPr = 0.25 * material.clearcoat;

    // Normalize probabilities
    float invTotalWt = 1.0 / (diffusePr + dielectricPr + metalPr + glassPr + clearCtPr);

    diffusePr *= invTotalWt;
    dielectricPr *= invTotalWt;
    metalPr *= invTotalWt;
    glassPr *= invTotalWt;
    clearCtPr *= invTotalWt;

    float cdf[5];
    cdf[0] = diffusePr;
    cdf[1] = cdf[0] + dielectricPr;
    cdf[2] = cdf[1] + metalPr;
    cdf[3] = cdf[2] + glassPr;
    cdf[4] = cdf[3] + clearCtPr;

    float r3 = Random();
    if (r3 < cdf[0]) //Diffuse
    {
        L = CosineSampleHemisphere(r1, r2);

    }
    else if (r3 < cdf[2])
    {
        vec3 H = SampleGGXVNDF(V, a_xy, vec2(r1, r2));
        float F = DielectricFresnel(abs(dot(V, H)), intersection.eta);

        if (H.z < 0.0)
            H = -H;

        // Rescale random number for reuse
        //Need to know the math behind it
        r3 = (r3 - cdf[2]) / (cdf[3] - cdf[2]);

        if (r3 < F)
        {
            L = normalize(reflect(-V, H));
        }
        else
        {
            L = normalize(refract(-V, H, intersection.eta));
        }
    }
    else //clearcoat
    {
        vec3 H = SampleGTR1(material.clearcoatGloss, r1, r2);
        if (H.z < 0.0)
            H = -H;

        L = normalize(reflect(-V, H));
    }

    L = ToWorld(T, B, N, L);
    V = ToWorld(T, B, N, V);


    return L;
    //return EvalDisney(intersection, material ,V, N, L, pdf);
}

