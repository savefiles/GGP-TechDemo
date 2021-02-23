#pragma once

#include "StandardIncludes.h"


class Transform
{
public:
	Transform();
	Transform(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT4 rotation);

	// Getters
	DirectX::XMFLOAT4 GetRotation();
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetScale();

	// Setters
	void SetRotation(DirectX::XMFLOAT4 newRotation);
	void SetPosition(DirectX::XMFLOAT3 newPosition);
	void SetScale(DirectX::XMFLOAT3 newScale);

	// Functions
	// Add to position ignoring the current orientation.
	void MoveAbsolute(float x, float y, float z);
	// Move position using current orientation
	void MoveRelative(float x, float y, float z);
	void Rotate(float pitch, float yaw, float roll);
	void Scale(float x, float y, float z);

	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT3 GetForwardVector();

private:
	DirectX::XMFLOAT4X4 worldMatrix;

	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
};


