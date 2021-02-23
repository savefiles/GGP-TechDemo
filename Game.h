#pragma once

#include "StandardIncludes.h"

#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Material.h"
#include "Light.h"
#include "Sky.h"
#include "Player.h"




class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Meshes that the entities will use.
	std::shared_ptr<Mesh> meshPlayer;
	std::shared_ptr<Mesh> meshCube;

	// Materials that the entities will use.
	//std::shared_ptr<Material> material1;
	//std::shared_ptr<Material> material2;
	//std::shared_ptr<Material> material3;

	// Textures that the materials will use.
	Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSSPtr;			// Sampler State Ptr
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRVPtr;	// Shader Resource View Ptr
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapSRVPtr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRVPtr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalnessSRVPtr;


	// Vector to hold five entities
	std::vector<std::shared_ptr<GameEntity>> entities;

	// Third person player
	std::unique_ptr<Player> player;

	// Lights
	std::vector<Light*> lights;
	std::vector<LightShaderInput> lightShaderInputs;

	// Skybox
	std::shared_ptr<Sky> skybox;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMap;




	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void DrawMesh(Mesh* mesh);




private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();

	
	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> normalMapPixelShader;
	std::shared_ptr<SimplePixelShader> PBRPixelShader; 		// Only need a pixel shader for PBR, vertex is the normal map shader.
	std::shared_ptr<SimplePixelShader> skyPixelShader;


	std::shared_ptr<SimpleVertexShader> normalMapVertexShader;
	std::shared_ptr<SimpleVertexShader> shadowVertexShader;			// Only need a vertex shader for shadows, output is directly used.
	std::shared_ptr<SimpleVertexShader> skyVertexShader;
};

