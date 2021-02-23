
#include "ShaderIncludes.hlsli"


// Constant buffer for data
cbuffer ExternalData : register(b0)
{
	float4x4 viewMatrix;   // The view matrix of the camera
	float4x4 projMatrix;   // The projection matrix of the camera
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixelSky main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixelSky output;

	// Remove the translation from the view matrix
	float4x4 noTranslationView = viewMatrix;
	noTranslationView._14 = 0;
	noTranslationView._24 = 0;
	noTranslationView._34 = 0;

	// Apply the projection and updated view matrix.
	matrix vp = mul(projMatrix, noTranslationView);		// NEEDS TO BE IN THIS ORDER, OR IT DOESN'T DRAW PROPERLY
	output.position = mul(vp, float4(input.position, 1.0f));

	// Set the depth of the output position to be 1.0f.
	output.position.z = output.position.w;

	// Pass through the sample direction.
	output.sampleDir = input.position;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}