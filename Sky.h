#pragma once
#include "StandardIncludes.h"
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

// Class that does everything skybox related.
class Sky
{
public:
	Sky(std::shared_ptr<Mesh> mesh, 
		ID3D11ShaderResourceView* cubeMap,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader,
		ID3D11SamplerState* samplerOptions, 
		ID3D11Device* device);

	void Draw(Camera* camera, ID3D11DeviceContext* context);


private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;			// Sampler options
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMap;			// Cube map SRV
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;	// Adjust depth buffer compare type
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerOptions;	// Rasterizer options

	std::shared_ptr<Mesh> skyGeometry;
	std::shared_ptr<SimplePixelShader> skyPixelShader;
	std::shared_ptr<SimpleVertexShader> skyVertexShader;

	void DrawMesh(Mesh* mesh, ID3D11DeviceContext* context);		// Ripped stright from Game.cpp

};

