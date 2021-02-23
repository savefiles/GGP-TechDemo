#include "GameEntity.h"
GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	materialPtr = material;
	meshPtr = mesh;
	transform = Transform();
}

Mesh* GameEntity::GetMesh()
{
	return meshPtr.get();
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

Material* GameEntity::GetMaterial()
{
	return materialPtr.get();
}
