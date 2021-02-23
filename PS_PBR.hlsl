
#include "ShaderIncludes.hlsli"


// Constant buffer
cbuffer ExternalData : register(b0)			// b = buffer register
{
	float numOfLights;

	Light lights[MAX_LIGHTS];
	
	float3 cameraPos;
}


Texture2D albedo			: register(t0); // t = texture register
Texture2D normalMap			: register(t1);
Texture2D roughnessMap		: register(t2);
Texture2D metalnessMap		: register(t3);
SamplerState samplerOptions : register(s0);	// s = sampler register



float3 ApplyNormalMap(float2 uv, float3 normal, float3 tangent) {
	// Sample and unpack the normal
	float3 unpackedNormal = normalMap.Sample(samplerOptions, uv).rgb * 2 - 1;

	// Create the TBN Matrix
	float3 N = normalize(normal);
	float3 T = normalize(tangent);
	T = normalize(T - N * dot(T, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Return the new normal.
	return mul(unpackedNormal, TBN);
}



// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixelNormalMap input) : SV_TARGET
{

	// Set the surface of the object to have a texture.
	// - Uncorrect the gamma of the sampled diffuse texture.
	float3 surfaceColor = pow(albedo.Sample(samplerOptions, input.uv).rgb, 2.2f);

	// Physically based rendering
	PBRinfo pbr;
	pbr.Roughness = roughnessMap.Sample(samplerOptions, input.uv).r;
	pbr.Metalness = metalnessMap.Sample(samplerOptions, input.uv).r;
	pbr.SpecularColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, pbr.Metalness);
	
	// Calculate the color of pixel based on the lights.
	input.normal = ApplyNormalMap(input.uv, input.normal, input.tangent);
	float3 lightPixelColorTotal = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < numOfLights; i++) {
		lightPixelColorTotal += GeneratePBRLightPixelColor(lights[i], pbr, surfaceColor, input.normal, input.worldPos, cameraPos);
	}

	// Calculate final pixel color and gamma correct
	float3 pixelColor = input.color.xyz * surfaceColor * lightPixelColorTotal;
	pixelColor = pow(pixelColor, 1.0f / 2.2f);

	return float4(pixelColor, 1);
}
