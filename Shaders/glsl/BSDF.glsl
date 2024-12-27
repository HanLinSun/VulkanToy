vec3 sampleLambertian(vec3 normal, inout float pdf) {
    onb uvw = Onb(normal);
    vec3 randomCos = random_cosine_direction();
    vec3 randomCosScattered = normalize(onbLocal(randomCos, uvw));
    float cosine = dot(normalize(normal), randomCosScattered);
    pdf = max(0.01, cosine) / pi;

    return randomCosScattered;
}