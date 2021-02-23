#pragma once

#include "StandardIncludes.h"
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <d3d11.h>
#include "Vertex.h"



class Mesh
{
public:

	// Constructor
	Mesh(Vertex vertexArray[], unsigned int indexArray[], int numOfVertices, int numOfIndices, ID3D11Device* device);
	Mesh(const char* pathToFile, ID3D11Device* device);

	// Destructor
	~Mesh();

	// Getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();

	// Functions
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indicies, int numIndices);
	void Draw(ID3D11DeviceContext* context);
	std::vector<Vertex>* GetVerticesWorldSpace(DirectX::XMFLOAT4X4 worldMatrix);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBufferPtr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferPtr;
	int m_numOfIndices;

	std::vector<Vertex> m_verts;
	std::vector<Vertex> m_vertsWorldSpace;
	void CreateBuffers(Vertex vertexArray[], unsigned int indexArray[], int numOfVertices, int numOfIndices, ID3D11Device* device);
};

