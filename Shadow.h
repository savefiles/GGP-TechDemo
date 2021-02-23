#pragma once
#include "StandardIncludes.h"
#include "SimpleShader.h"
#include "GameEntity.h"
#include "Light.h"

class Shadow
{
private:
	// Matricies for the shadow map
	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projMatrix;

	// ComPtrs for all of the DirectX vars
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

	// Vertex Shader used to draw the shadows (no PS needed)
	std::shared_ptr<SimpleVertexShader> m_vertexShader;

	// Window size;
	int m_width;
	int m_height;

	// Other
	int m_shadowMapSize;		// Ideally a power of 2.

public:
	Shadow(ID3D11Device* device, std::shared_ptr<SimpleVertexShader> vertexShader, int windowWidth, int windowHeight, int shadowMapSize = 1024);
	
	void Draw(std::vector<std::shared_ptr<GameEntity>> entities,
		std::vector<Light*> lights,
		ID3D11RenderTargetView** backBufferRTV,
		ID3D11DepthStencilView* depthStencilView,
		ID3D11DeviceContext* context);

	void OnWindowResize(int width, int height);


	// Getters.
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjMatrix();
};

