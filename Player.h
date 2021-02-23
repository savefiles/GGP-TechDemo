#pragma once

#include "GameEntity.h"
#include "Camera.h"

class Player
{
private:
	// Objects describing this player
	std::shared_ptr<GameEntity> m_gameEntity;
	std::unique_ptr<Camera> m_camera;

	// Vars
	float m_movementSpeed = 4.0f;
public:
	Player(DirectX::XMFLOAT3 position, std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, float windowAspectRatio);
	void Update(float dt, HWND windowHandle);
	void Teleport(DirectX::XMFLOAT3 position);
	DirectX::BoundingBox GetMinMaxARBB();

	// Getters and setters
	Camera* GetCamera();
	std::shared_ptr<GameEntity> GetGameEntity();
};

