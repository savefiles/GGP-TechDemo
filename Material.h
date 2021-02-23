#pragma once

#include "StandardIncludes.h"
#include "SimpleShader.h"

class Material
{
public:
	// No normal map constructor
	Material(DirectX::XMFLOAT4 colorTint, 
		     int specular, 
		     std::shared_ptr<SimplePixelShader> pixelShader, 
		     std::shared_ptr<SimpleVertexShader> vertexShader,
			 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRVPtr,
			 Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSSPtr,
			 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapSRVPtr = nullptr,
			 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRVPtr = nullptr,
			 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalnessSRVPtr = nullptr);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	int GetSpecularExponent();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureSRVComPtr();
	Microsoft::WRL::ComPtr<ID3D11SamplerState> GetTextureSSComPtr();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetNormalMapSRVComPtr();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetRoughnessSRVComPtr();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetMetalnessSRVComPtr();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 colorTint);


private:
	DirectX::XMFLOAT4 colorTint;
	int specularExponent;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapSRVPtr;	// Normal Map Shader Resource View Ptr
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRVPtr;		// Texture (Albedo) Shader Resource View Ptr
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRVPtr;	// Roughness SRV Ptr (zero if not PBR)
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalnessSRVPtr;	// Metalness SRV Ptr (zero if not PBR)
	Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSSPtr;			// Sampler State Ptr
};

