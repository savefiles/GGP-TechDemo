#include "Transform.h"
using namespace DirectX;

Transform::Transform()
{
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
}

Transform::Transform(XMFLOAT3 position, XMFLOAT3 scale, XMFLOAT4 rotation)
{
	this->position = position;
	this->scale = scale;
	this->rotation = rotation;
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
}


#pragma region Getters/Setters
XMFLOAT4 Transform::GetRotation() { return rotation; }
XMFLOAT3 Transform::GetPosition() { return position; }
XMFLOAT3 Transform::GetScale() { return scale; }

void Transform::SetRotation(XMFLOAT4 newRotation) { rotation = newRotation; }
void Transform::SetPosition(XMFLOAT3 newPosition) { position = newPosition; }
void Transform::SetScale(XMFLOAT3 newScale) { scale = newScale; }
#pragma endregion


#pragma region Functions
void Transform::MoveAbsolute(float x, float y, float z) { XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), XMVectorSet(x, y, z, 0.0f))); }
void Transform::MoveRelative(float x, float y, float z) {
	XMStoreFloat3(
		&position,
		XMVectorAdd(
			XMLoadFloat3(&position),
			XMVector3Rotate(
				XMVectorSet(x, y, z, 0.0f),
				XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z))));
}
void Transform::Rotate(float pitch, float yaw, float roll) { XMStoreFloat4(&rotation, XMVectorAdd(XMLoadFloat4(&rotation), XMVectorSet(pitch, yaw, roll, 0.0f))); }
void Transform::Scale(float x, float y, float z) { XMStoreFloat3(&scale, XMVectorMultiply(XMLoadFloat3(&scale), XMVectorSet(x, y, z, 0.0f))); };

XMFLOAT4X4 Transform::GetWorldMatrix() {
	// Get the quats for position, rotation, and scale.
	XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX scalingMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
	
	// Store the world matrix and return it.
	XMStoreFloat4x4(&worldMatrix, scalingMatrix * rotationMatrix * translationMatrix);
	return worldMatrix;
}
XMFLOAT3 Transform::GetForwardVector() {
	XMFLOAT3 forwardVec = XMFLOAT3(0, 0, 0);
	XMStoreFloat3(
		&forwardVec,
		XMVector3Rotate(
			XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
	return forwardVec;
}
#pragma endregion
