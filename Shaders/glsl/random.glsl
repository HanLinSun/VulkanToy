//In Compute Shader cannot use std::srand, so must implement random number generation method
//based on pcg_setseq_32_step_r in pcg-c, using inc = 17
uint StepRandomGeneration(uint rngState)
{
	return rngState * 747796405 +17;
}

float StepAndOutputRNGFloat(inout uint rngState)
{
	// Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
	rngState = StepRandomGeneration(rngState);
	uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
	word = (word >> 22) ^ word;
	return float(word) / 4294967295.0f;
}

uint rngState = (577 * gl_GlobalInvocationID.x + gl_GlobalInvocationID.y);

float Random()
{
	return StepAndOutputRNGFloat(rngState);
}

float Random(float min, float max) {
	// Returns a random real in [min,max).
	return min + (max - min) * Random();
}

vec3 RandomInUnitSphere() {
	vec3 p = vec3(Random(-0.4, 0.4), Random(-0.4, 0.4), Random(-0.4, 0.4));
	return normalize(p);
}

vec3 RandomInHemisphere(vec3 normal)
{
	vec3 inUnitSphereVec = RandomInUnitSphere();
	if (dot(inUnitSphereVec, normal) > 0.0)
	{
		return inUnitSphereVec;
	}
	else return -inUnitSphereVec;
}

double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}