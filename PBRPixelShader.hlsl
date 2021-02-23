
#include "ShaderIncludes.hlsli"


// Constant buffer
cbuffer ExternalData : register(b0)			// b = buffer register
{
	Light light1;
	Light light2;
	Light light3;
	float3 cameraPos;
}

Texture2D albedo			: register(t0); // t = texture register
Texture2D roughnessMap		: register(t1);
Texture2D metalnessMap		: register(t2);
SamplerState samplerOptions : register(s0);	// s = sampler register



// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
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
	float3 lightPixelColor1 = GeneratePBRLightPixelColor(light1, pbr, surfaceColor, input.normal, input.worldPos, cameraPos);
	float3 lightPixelColor2 = GeneratePBRLightPixelColor(light2, pbr, surfaceColor, input.normal, input.worldPos, cameraPos);
	float3 lightPixelColor3 = GeneratePBRLightPixelColor(light3, pbr, surfaceColor, input.normal, input.worldPos, cameraPos);
	float3 lightPixelColorTotal = lightPixelColor1 + lightPixelColor2 + lightPixelColor3;


	// Calculate final pixel color and gamma correct
	float3 pixelColor = input.color.xyz * surfaceColor * lightPixelColorTotal;
	pixelColor = pow(pixelColor, 1.0f / 2.2f);

	return float4(pixelColor, 1);
}

