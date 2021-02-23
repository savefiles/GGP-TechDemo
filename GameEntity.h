#pragma once

#include "StandardIncludes.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	Mesh* GetMesh();
	Transform* GetTransform();
	Material* GetMaterial();

private:
	Transform transform;
	std::shared_ptr<Mesh> meshPtr;
	std::shared_ptr<Material> materialPtr;
};

