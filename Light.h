#pragma once

#include "StandardIncludes.h"
#include "Transform.h"

#define MAX_LIGHTS 16

// Enums
enum class LightType { Directional, Point, Spot };

// Structs
// The struct that the pixel shader uses for lights.
struct LightShaderInput {
	int lightType;
	DirectX::XMFLOAT3 diffuseColor;
	float padding1;
	DirectX::XMFLOAT3 ambientColor;
	float padding2;
	DirectX::XMFLOAT3 direction;
	float spotFalloff;
	DirectX::XMFLOAT3 position;
};
struct ViewAndProjMatrices {
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Proj;
};

class Light
{
private:
	// Base light information.
	vec3 m_diffuse;
	vec3 m_ambient;
	vec3 m_direction;
	vec3 m_position;
	int m_lightType;
	float m_theta;
	float m_spotFalloff = 0.0f;
	float m_spotPower = 1.0f;

	// Swinging light information.
	bool m_isSwinging = false;
	DirectX::XMVECTOR m_swingInitDir;
	DirectX::XMVECTOR m_swingFinalDir;
	float m_swingOmega = 0.0f;			// Calculated angle between the two directions.
	float m_swingCurrPercent = 0.0f;	// Percentage to get from init -> final -> init.
	float m_swingDuration = 1.0f;		// Time it takes to get from initial to final.

	// Matrix information
	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projMatrix;

	// Functions
	void InitializeLight(vec3 diffuse, vec3 ambient, vec3 direction, vec3 position);

	// Structs


public:
	// Directional or point light (use other constructor for spot lights!).
	Light(vec3 diffuse, vec3 ambient, vec3 directionOrPosition, LightType type);
	// Spot light.
	Light(vec3 diffuse, vec3 ambient, vec3 position, vec3 direction, float spotFalloff, float spotPower);
	// Update the lights (only really relevant for moving/swinging lights).
	void Update(float dt);
	// Output a struct for use with shaders.
	LightShaderInput Output();
	// Output a struct that contains proj and view matrices.
	ViewAndProjMatrices GetMatrices();
	// Allows the light to rotate.
	void ConvertToSwinging(float rel_roll, float rel_pitch, float rel_yaw, float swingTime);

	// Allows the light to move.

	// Check a triangle against this light for collision.
	bool IsBoxCollidingSpotlight(DirectX::BoundingBox box);
};

