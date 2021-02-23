#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__


// -------------------------------------------------------------- //
// Constants
// -------------------------------------------------------------- //


// The fresnel value for non-metals (dielectrics)
// Page 9: "F0 of nonmetals is now a constant 0.04"
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f;

// Handy to have this as a constant
static const float PI = 3.14159265359f;

// Maximum number of lights and objects for the scene.
#define MAX_LIGHTS 128
#define MAX_OBJECTS 256



// -------------------------------------------------------------- //
// Pipeline Includes
// -------------------------------------------------------------- //

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float3 normal		: NORMAL;		// Normal the point
	float2 uv			: TEXCOORD;
	float3 tangent		: TANGENT;		// Tangent to the UV
};

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float4 color		: COLOR;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float4 specular		: COLOR1;
	//float4 posForShadow : SHADOWS;
};

// Struct representing the data we expect to receive from earlier pipeline stages FOR NORMAL MAP SHADERS
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage

struct VertexToPixelNormalMap
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float4 color		: COLOR;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD;
	float3 worldPos		: POSITION;
	float4 specular		: COLOR1;
	float3 tangent		: TANGENT;
	//float4 posForShadow[MAX_OBJECTS] : SHADOWS;
};


struct VertexToPixelSky
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 sampleDir	: DIRECTION;
};




// -------------------------------------------------------------- //
// Vertex Shader Includes
// -------------------------------------------------------------- //






// -------------------------------------------------------------- //
// Pixel Shader Includes
// -------------------------------------------------------------- //

// Struct representing the data we expect for a directional light.
// - Semantics aren't necessary as it isn't a shader input/output struct.
// - The struct DOES NOT have automatic padding.
struct Light
{
	int    LightType;
	float3 DiffuseColor;

	float  padding1;
	float3 AmbientColor;

	float  padding2;
	float3 Direction;

	float  SpotFalloff;
	float3 Position;
};

// Struct representing information needed for physically based rendering.
struct PBRinfo {
	float Roughness;
	float Metalness;
	float3 SpecularColor;
};


// Helper function to generate the specular.
float3 GeneratePhongSpecular(float3 lightDirection, float3 normal, float3 worldPos, float4 specular, float3 cameraPos)
{
	lightDirection = -lightDirection;
	float3 V = normalize(cameraPos - worldPos);
	float3 R = reflect(lightDirection, normalize(normal));
	float spec = pow(saturate(dot(R, V)), specular.x);
	return float3(spec, spec, spec);
}

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
	return saturate(dot(normal, dirToLight));
}


// Calculates diffuse amount based on energy conservation
//
// diffuse - Diffuse amount
// specular - Specular color (including light color)
// metalness - surface metalness amount
//
// Metals should have an albedo of (0,0,0)...mostly
// See slide 65: http://blog.selfshadow.com/publications/s2014-shading-course/hoffman/s2014_pbs_physics_math_slides.pdf
float3 DiffuseEnergyConserve(float diffuse, float3 specular, float metalness)
{
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

// GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness)
{
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}



// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n (full specular color)
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}


// Geometric Shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2, roughness remapped to (r+1)/2
//
// n - Normal
// v - View vector
//
// G(l,v,h)
float GeometricShadowing(float3 n, float3 v, float3 h, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}



// Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
//
// D() - Spec Dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float metalness, float3 specColor)
{
	// Other vectors
	float3 h = normalize(v + l);

	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, h, roughness) * GeometricShadowing(n, l, h, roughness);

	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}

// Main function to generate lights and reflections for non-PBR materials.
float3 GeneratePhongLightPixelColor(Light dl, float3 normal, float3 worldPos, float4 specular, float3 cameraPos)
{
	// Normalize the input normal
	float3 normalNormal = normalize(normal);

	// Calculate the amount of light
	float3 lightDirection;
	if (dl.LightType == 0) // If it's a directional light
	{
		lightDirection = -dl.Direction;
	}
	else				// It's a point light
	{
		lightDirection = dl.Position - worldPos;
	}
	lightDirection = normalize(lightDirection);

	float3 specularRes = GeneratePhongSpecular(lightDirection, normal, worldPos, specular, cameraPos);

	// Calculate the amount of light
	float3 lightIntensity = saturate(dot(normalNormal, -lightDirection));
	if (dl.LightType == 2) {	// It's a spot light
		lightIntensity = mul(lightIntensity, pow((max(dot(-lightDirection, -dl.Direction), 0.0f)), dl.SpotFalloff));
	}

	// Cut specular if there is no light shining on object.
	specularRes *= any(lightIntensity);

	// Calculate the per pixel color of the light.
	float3 lightPixelColor = lightIntensity * dl.DiffuseColor + dl.AmbientColor + specularRes;

	return lightPixelColor;
}

// Main function to generate lights and reflections for PBR materials.
float3 GeneratePBRLightPixelColor(Light dl, PBRinfo pbr, float3 surfaceColor, float3 normal, float3 worldPos, float3 cameraPos)
{
	// Normalize the input normal
	float3 normalNormal = normalize(normal);

	// Calculate the amount of light
	float3 toLight;
	if (dl.LightType == 0) // If it's a directional light
	{
		toLight = -dl.Direction;
	}
	else				// It's a point or spot light
	{
		toLight = dl.Position - worldPos;
	}
	toLight = normalize(toLight);

	// Cook-Torrence lighting (replaced Phong)
	float3 v = normalize(cameraPos - worldPos);		// Vector to the camera
	float3 specularRes = MicrofacetBRDF(normal, toLight, v, pbr.Roughness, pbr.Metalness, pbr.SpecularColor);
	float diffuse = DiffusePBR(normal, toLight);

	// Energy conservation
	float3 balancedDiffuse = DiffuseEnergyConserve(diffuse, specularRes, pbr.Metalness);

	// Calculate the amount of light
	float3 lightIntensity = saturate(dot(normalNormal, toLight));
	if (dl.LightType == 2) {	// It's a spot light
		lightIntensity = mul(lightIntensity, pow((max(dot(-toLight, dl.Direction), 0.0f)), 25.0f));
	}

	// Cut specular if its diffuse contribution is zero.
	// specularRes *= any(dl.DiffuseColor);

	// Calculate the per pixel color of the light.
	// float3 lightPixelColor = (balancedDiffuse * dl.AmbientColor + specularRes) * lightIntensity * dl.DiffuseColor;
	float3 lightPixelColor = (balancedDiffuse * surfaceColor + specularRes) * lightIntensity * dl.DiffuseColor;

	return lightPixelColor;
}

// Main function to generate lights and reflections for PBR materials, assuming the light is a spot light..
float3 GeneratePBRLightPixelColorSpot(Light dl, PBRinfo pbr, float3 surfaceColor, float3 normal, float3 worldPos, float3 cameraPos)
{
	// Normalize the input normal
	float3 normalNormal = normalize(normal);

	// Calculate the amount of light
	float3 toLight = normalize(dl.Position - worldPos);

	// Cook-Torrence lighting (replaced Phong)
	float3 v = normalize(cameraPos - worldPos);		// Vector to the camera
	float3 specularRes = MicrofacetBRDF(normal, toLight, v, pbr.Roughness, pbr.Metalness, pbr.SpecularColor);
	float diffuse = DiffusePBR(normal, toLight);

	// Energy conservation
	float3 balancedDiffuse = DiffuseEnergyConserve(diffuse, specularRes, pbr.Metalness);

	// Calculate the amount of light
	float3 lightIntensity = saturate(dot(normalNormal, toLight));
	lightIntensity = mul(lightIntensity, pow((max(dot(-toLight, dl.Direction), 0.0f)), dl.SpotFalloff));


	// Cut specular if its diffuse contribution is zero.
	// specularRes *= any(dl.DiffuseColor);

	// Calculate the per pixel color of the light.
	// float3 lightPixelColor = (balancedDiffuse * dl.AmbientColor + specularRes) * lightIntensity * dl.DiffuseColor;
	float3 lightPixelColor = (balancedDiffuse * surfaceColor + specularRes) * lightIntensity * dl.DiffuseColor;

	return lightPixelColor;
}





#endif