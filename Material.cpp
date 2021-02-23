#include "Material.h"

Material::Material(DirectX::XMFLOAT4 colorTint,
				   int specular,
				   std::shared_ptr<SimplePixelShader> pixelShader,
				   std::shared_ptr<SimpleVertexShader> vertexShader,
				   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRVPtr,
				   Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSSPtr,
				   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapSRVPtr,
				   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRVPtr,
				   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalnessSRVPtr)
{
	this->colorTint = colorTint;
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
	this->specularExponent = specular;
	this->textureSRVPtr = textureSRVPtr;
	this->textureSSPtr = textureSSPtr;
	this->normalMapSRVPtr = normalMapSRVPtr;
	this->roughnessSRVPtr = roughnessSRVPtr;
	this->metalnessSRVPtr = metalnessSRVPtr;
}

DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader(){ return pixelShader; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader(){ return vertexShader; }
int Material::GetSpecularExponent(){ return specularExponent; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetTextureSRVComPtr() { return textureSRVPtr; }
Microsoft::WRL::ComPtr<ID3D11SamplerState> Material::GetTextureSSComPtr() { return textureSSPtr; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetNormalMapSRVComPtr() { return normalMapSRVPtr; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetRoughnessSRVComPtr() { return roughnessSRVPtr; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetMetalnessSRVComPtr() { return metalnessSRVPtr; }


void Material::SetColorTint(DirectX::XMFLOAT4 colorTint) { this->colorTint = colorTint; }
