#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 position, float aspectRatio,
			   float fovAngle, 
			   float nearPlaneDist, 
			   float farPlaneDist, 
			   float movementSpeed, 
			   float mouseLookSpeed) {

	this->fovAngle = fovAngle; // default to 120 degrees
	this->nearPlaneDist = nearPlaneDist;
	this->farPlaneDist = farPlaneDist;
	this->movementSpeed = movementSpeed;
	this->mouseLookSpeed = mouseLookSpeed;

	transform = Transform(position, DirectX::XMFLOAT3(1, 1, 1), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));

	UpdateProjectionMatrix(aspectRatio);
	UpdateViewMatrix();
}

// Getters
DirectX::XMFLOAT4X4 Camera::GetViewMatrix() { return viewMat; }
DirectX::XMFLOAT4X4 Camera::GetProjMatrix() { return projMat; }
Transform Camera::GetTransform() { return transform; }

void Camera::UpdateProjectionMatrix(float aspectRatio) {
	XMStoreFloat4x4(&projMat, DirectX::XMMatrixPerspectiveFovLH(fovAngle, aspectRatio, nearPlaneDist, farPlaneDist));
}

void Camera::UpdateViewMatrix() {
	DirectX::XMStoreFloat4x4(
		&viewMat, 
		DirectX::XMMatrixLookToLH(
			DirectX::XMLoadFloat3(&transform.GetPosition()),
			DirectX::XMLoadFloat3(&transform.GetForwardVector()),
			DirectX::XMVectorSet(0, 1, 0, 0)));
}

void Camera::Update(float dt, HWND windowHandle) {
	
	// Key inputs (controlled by player now)
	/*
	if (GetAsyncKeyState('W') & 0x8000) { transform.MoveRelative(0, 0, movementSpeed * dt); }
	if (GetAsyncKeyState('S') & 0x8000) { transform.MoveRelative(0, 0, -movementSpeed * dt); }
	if (GetAsyncKeyState('A') & 0x8000) { transform.MoveRelative(-movementSpeed * dt, 0, 0); }
	if (GetAsyncKeyState('D') & 0x8000) { transform.MoveRelative(movementSpeed * dt, 0, 0); }

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) { transform.MoveAbsolute(0, movementSpeed * dt, 0); }
	if (GetAsyncKeyState('X') & 0x8000) { transform.MoveAbsolute(0, -movementSpeed * dt, 0); }
	*/

	// Get the mouse position relative to the screen.
	POINT mousePos = {};
	GetCursorPos(&mousePos);
	ScreenToClient(windowHandle, &mousePos);

	// Should we even check mouse positions.
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		// Calculate difference in mouse pos, and rotate accordingly.
		// Limit the camera to certain angles.
		DirectX::XMFLOAT4 rotation = transform.GetRotation();
		
		float xDiff = (float) mousePos.x - mousePosPrev.x;
		float yDiff = (float) mousePos.y - mousePosPrev.y;

		xDiff = xDiff * dt * mouseLookSpeed;
		yDiff = yDiff * dt * mouseLookSpeed;

		if (rotation.x > 0.66f) yDiff = min(yDiff, 0.0f);
		if (rotation.x < -0.66f) yDiff = max(yDiff, 0.0f);
		if (rotation.y > 0.5f) xDiff = min(xDiff, 0.0f);
		if (rotation.y < -0.5f) xDiff = max(xDiff, 0.0f);

		transform.Rotate(yDiff, xDiff, 0.0f);
	}

	// Update for the next frame.
	mousePosPrev = mousePos;
	UpdateViewMatrix();
}

void Camera::SetPositionAbsolute(DirectX::XMFLOAT3 newPosition) {
	transform.SetPosition(newPosition);
}