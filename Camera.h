#pragma once

#include "Transform.h"

// Defs
#define PI (2.0f * acos(0.0f))


class Camera
{
public:
	// Parameterized constructor.
	Camera(DirectX::XMFLOAT3 position, float aspectRatio,
		float fovAngle = PI / 4.0f,
		float nearPlaneDist = 0.1f,
		float farPlaneDist = 100.0f,
		float movementSpeed = 5.0f,
		float mouseLookSpeed = 0.3f);

	// Getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjMatrix();
	Transform GetTransform();

	// Functions
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt, HWND windowHandle);
	void SetPositionAbsolute(DirectX::XMFLOAT3 newPosition);

private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMat;
	DirectX::XMFLOAT4X4 projMat;
	POINT mousePosPrev;

	float fovAngle;
	float nearPlaneDist;
	float farPlaneDist;
	float movementSpeed;
	float mouseLookSpeed;
};
