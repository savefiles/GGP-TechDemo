
#include "ShaderIncludes.hlsli"


// Constant buffer for matrix information (from the light)
cbuffer ExternalData : register(b0)
{
	matrix worldMatrix;	// The world matrix of the ENTITY
	matrix viewMatrix;	// The view matrix of the LIGHT
	matrix projMatrix;	// The projection matrix of the LIGHT
}

// This output gets used directly, no pixel shader involved.
struct VertexShadowOutput 
{
	float4 position		: SV_POSITION;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexShadowOutput main(VertexShaderInput input)
{
	// Set up output struct
	VertexShadowOutput output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in future assignments).
	matrix wvp = mul(projMatrix, mul(viewMatrix, worldMatrix));
	output.position = mul(wvp, float4(input.position, 1.0f));

	// Nothing else is needed for the shadows.
	//// Calculate the world position of the vertex.
	//output.worldPos = (float3) mul(worldMatrix, float4(input.position, 1.0f));

	//// Pass the normal and tangent vector through with minor changes
	//// - Transform it using the world matrix
	//// - Since we don't care about translations, cast as a 3x3
	//output.normal = mul((float3x3) worldMatrix, input.normal);
	//output.tangent = normalize(mul((float3x3) worldMatrix, input.tangent));

	//// Pass the color, specular, and uv through
	//output.color = colorTint;
	//output.specular = specular;
	//output.uv = input.uv;


	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}