#include "Player.h"

using namespace DirectX;

Player::Player(DirectX::XMFLOAT3 position, std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, float windowAspectRatio)
{
	m_gameEntity = std::make_unique<GameEntity>(mesh, material);
	m_gameEntity->GetTransform()->Scale(0.005f, 0.005f, 0.005f);
	m_gameEntity->GetTransform()->SetPosition(position);
	m_gameEntity->GetTransform()->Rotate(0, PI, 0);
	m_camera = std::make_unique<Camera>(position, windowAspectRatio);
}

void Player::Update(float dt, HWND windowHandle)
{
	Transform* transform = m_gameEntity->GetTransform();
	// printf("(%4.8f, %4.8f, %4.8f)\n", m_camera->GetTransform().GetRotation().x, m_camera->GetTransform().GetRotation().y, m_camera->GetTransform().GetRotation().z);

	// Key inputs
	if (GetAsyncKeyState('W') & 0x8000) { transform->MoveRelative(0, 0, -m_movementSpeed * dt); }
	if (GetAsyncKeyState('S') & 0x8000) { transform->MoveRelative(0, 0, m_movementSpeed * dt); }
	if (GetAsyncKeyState('A') & 0x8000) { transform->MoveRelative(m_movementSpeed * dt, 0, 0); }
	if (GetAsyncKeyState('D') & 0x8000) { transform->MoveRelative(-m_movementSpeed * dt, 0, 0); }

	// Move the camera.
	XMFLOAT3 pPos = transform->GetPosition();
	XMFLOAT3 newCameraPos = XMFLOAT3(pPos.x, pPos.y + 5.0f, pPos.z - 5.0f);
	m_camera->SetPositionAbsolute(newCameraPos);

	// Update the camera.
	m_camera->Update(dt, windowHandle);

}

void Player::Teleport(DirectX::XMFLOAT3 position)
{
	// Move player
	m_gameEntity->GetTransform()->SetPosition(position);

	// Move camera
	m_camera->SetPositionAbsolute(XMFLOAT3(position.x, position.y + 5.0f, position.z - 5.0f));
	m_camera->UpdateViewMatrix();
}



BoundingBox Player::GetMinMaxARBB()
{
	XMFLOAT4X4 worldMatrix = m_gameEntity->GetTransform()->GetWorldMatrix();
	std::vector<Vertex>* worldVerts = m_gameEntity->GetMesh()->GetVerticesWorldSpace(worldMatrix);

	// Test for the largest/smallest.
	vec3 minVec = (*worldVerts)[0].Position;
	vec3 maxVec = (*worldVerts)[0].Position;

	for (int i = 1; i < worldVerts->size(); i++) {
		if (maxVec.x < (*worldVerts)[i].Position.x) maxVec.x = (*worldVerts)[i].Position.x;
		if (maxVec.y < (*worldVerts)[i].Position.y) maxVec.y = (*worldVerts)[i].Position.y;
		if (maxVec.z < (*worldVerts)[i].Position.z) maxVec.z = (*worldVerts)[i].Position.z;

		if ((*worldVerts)[i].Position.x < minVec.x) minVec.x = (*worldVerts)[i].Position.x;
		if ((*worldVerts)[i].Position.y < minVec.y) minVec.y = (*worldVerts)[i].Position.y;
		if ((*worldVerts)[i].Position.z < minVec.z) minVec.z = (*worldVerts)[i].Position.z;
	}
	vec3 center; vec3 extents;
	XMStoreFloat3(&center, (XMLoadFloat3(&maxVec) + XMLoadFloat3(&minVec)) / 2.0f);
	XMStoreFloat3(&extents, (XMLoadFloat3(&maxVec) - XMLoadFloat3(&minVec)) / 2.0f);

	return BoundingBox { center, extents };
}


Camera* Player::GetCamera() { return m_camera.get(); }
std::shared_ptr<GameEntity> Player::GetGameEntity() { return m_gameEntity; }
