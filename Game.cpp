#include "Game.h"
#include "Vertex.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Initialize and create the light.
	lights = std::vector<Light*>();
	lights.push_back(new Light(
		vec3(1.0f, 1.0f, 1.0f),		// Diffuse
		vec3(1.0f, 1.0f, 1.0f),		// Ambient
		vec3(0.0f, 3.0f, -5.0f),		// Position
		vec3(0.0f, -1.0f, 0.0f),	// Direction
		4.0f,						// Spot falloff
		7.0f));						// Light power

	lights.push_back(new Light(
		vec3(1.0f, 1.0f, 1.0f),		
		vec3(1.0f, 1.0f, 1.0f),		
		vec3(0.0f, 3.0f, 7.5f),		
		vec3(0.0f, -1.0f, -0.5773f),
		50.0f,
		7.0f));						
	lights[1]->ConvertToSwinging(-60.0f, 0.0f, 0.0f, 2.0f);

	lights.push_back(new Light(
		vec3(1.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f),
		vec3(0.0f, 3.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.5773f),
		50.0f,
		7.0f));
	lights[2]->ConvertToSwinging(60.0f, 0.0f, 0.0f, 2.0f);

	lights.push_back(new Light(
		vec3(1.0f, 1.0f, 1.0f),		// Diffuse
		vec3(1.0f, 1.0f, 1.0f),		// Ambient
		vec3(0.0f, 3.0f, 12.5f),		// Position
		vec3(0.0f, -1.0f, 0.0f),	// Direction
		4.0f,						// Spot falloff
		7.0f));						// Light power

	//lights.resize(MAX_LIGHTS);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	normalMapVertexShader = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VS_Normal.cso").c_str());
	normalMapPixelShader = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PS_Normal.cso").c_str());

	PBRPixelShader = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PS_PBR.cso").c_str());

	skyVertexShader = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VS_Sky.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"PS_Sky.cso").c_str());

	// shadowVertexShader = std::make_shared<SimpleVertexShader>(device.Get(), context.Get(), GetFullPathTo_Wide(L"VS_Shadow.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 colors[3] = { red, green, blue };

	// Struct that holds data about a mesh.
	struct MeshData {
		std::vector<Vertex> vertices;
		int verticesSize = 0;
		std::vector<unsigned int> indices;
		int indicesSize = 0;
	};

	D3D11_SAMPLER_DESC textureSSDesc1;
	textureSSDesc1.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		// Defines how to handle addresses out of the UV range of 0 - 1.
	textureSSDesc1.AddressV = textureSSDesc1.AddressU;
	textureSSDesc1.AddressW = textureSSDesc1.AddressU;
	textureSSDesc1.Filter = D3D11_FILTER_ANISOTROPIC;			// Sampling (currently Anisotropic filtering)
	textureSSDesc1.MaxAnisotropy = 4;
	textureSSDesc1.MaxLOD = D3D11_FLOAT32_MAX;					// Mipmapping (currently set to always mipmap).
	textureSSDesc1.MipLODBias = 0.0f;

	device.Get()->CreateSamplerState(&textureSSDesc1, textureSSPtr.GetAddressOf());

	// Create the textures.
	// - First point the SRV to the texture file.
	// - Create a sampler description to use in the sampler state
	// - Create the sampler state.
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bronze/bronze_albedo.png").c_str(), nullptr, textureSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bronze/bronze_normals.png").c_str(), nullptr, normalMapSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bronze/bronze_roughness.png").c_str(), nullptr, roughnessSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bronze/bronze_metal.png").c_str(), nullptr, metalnessSRVPtr.GetAddressOf());
	std::shared_ptr<Material> matBronze = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 50, PBRPixelShader, normalMapVertexShader, textureSRVPtr, textureSSPtr,
		normalMapSRVPtr, roughnessSRVPtr, metalnessSRVPtr);

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bricks/bricks_albedo.jpg").c_str(), nullptr, textureSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bricks/bricks_normals.jpg").c_str(), nullptr, normalMapSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Bricks/bricks_roughness.jpg").c_str(), nullptr, roughnessSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/nonmetal.png").c_str(), nullptr, metalnessSRVPtr.GetAddressOf());
	std::shared_ptr<Material> matBrick = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 20, PBRPixelShader, normalMapVertexShader, textureSRVPtr, textureSSPtr,
		normalMapSRVPtr, roughnessSRVPtr, metalnessSRVPtr);

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Metals/Metal015_2K_Color.jpg").c_str(), nullptr, textureSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Metals/Metal015_2K_Normal.jpg").c_str(), nullptr, normalMapSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Metals/Metal015_2K_Roughness.jpg").c_str(), nullptr, roughnessSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Metals/Metal015_2K_Metalness.png").c_str(), nullptr, metalnessSRVPtr.GetAddressOf());
	std::shared_ptr<Material> matMetal1 = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 100, PBRPixelShader, normalMapVertexShader, textureSRVPtr, textureSSPtr,
		normalMapSRVPtr, roughnessSRVPtr, metalnessSRVPtr);

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Snowman/Snowman_Albedo.png").c_str(), nullptr, textureSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Snowman/Snowman_Normals.png").c_str(), nullptr, normalMapSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Snowman/Snowman_Roughness.png").c_str(), nullptr, roughnessSRVPtr.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/nonmetal.png").c_str(), nullptr, metalnessSRVPtr.GetAddressOf());
	std::shared_ptr<Material> matSnowman = std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 100, PBRPixelShader, normalMapVertexShader, textureSRVPtr, textureSSPtr,
		normalMapSRVPtr, roughnessSRVPtr, metalnessSRVPtr);



	// Create the meshes from a .obj file.
	meshPlayer = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/SnowmanOBJ.obj").c_str(), device.Get());
	meshCube = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device.Get());



	// Create the player
	float aspectRatio = (float)this->width / this->height;
	player = std::make_unique<Player>(XMFLOAT3(0.0f, 0.0f, -5.0f), meshPlayer, matSnowman, aspectRatio);
	entities.push_back(player->GetGameEntity());

	// Create the PBR materials
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 5; j++) {
			std::shared_ptr<GameEntity> floor = std::make_shared<GameEntity>(meshCube, matMetal1);
			floor->GetTransform()->MoveAbsolute(20.0f * (i - 1), -0.3f, 20.0f * j);
			floor->GetTransform()->Scale(20.0f, 0.1f, 20.0f);
			entities.push_back(floor);
		}
	}

	// Load the cube map
	CreateDDSTextureFromFile(device.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/Skies/SpaceCubeMap.dds").c_str(), nullptr, cubeMap.GetAddressOf());
	skybox = std::make_shared<Sky>(meshCube, cubeMap.Get(), skyVertexShader, skyPixelShader, textureSSPtr.Get(), device.Get());
	
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Redo the projection matrix of the camera (if the camera exists)
	if (player) {
		float aspectRatio = (float)this->width / this->height;
		player->GetCamera()->UpdateProjectionMatrix(aspectRatio);
	}

}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	// Update the player
	player->Update(deltaTime, this->hWnd);

	// Update the lights.
	lightShaderInputs.clear();
	for (int i = 0; i < lights.size(); i++) {
		lights[i]->Update(deltaTime);
		lightShaderInputs.push_back(lights[i]->Output());
	}

	//// Check to see if the player is in a light or not.
	bool isInLight = false;
	BoundingBox playerBB = player->GetMinMaxARBB();
	for (int j = 0; j < lights.size(); j++) {
		if (lights[j]->IsBoxCollidingSpotlight(playerBB)) {
			//printf("It is colliding, time = %4.2f\n", totalTime);
			isInLight = true;
			break;
		}
	}
	if (!isInLight) {
		player->Teleport(XMFLOAT3(0.0f, 0.0f, -5.0f));
		//printf("Is not colliding, time = %4.2f\n", totalTime);
	}


	// Transform the entities
	/*
	// Make the top left game entity spinnnnnn (around the origin)
	entities[0]->GetTransform()->Rotate(0.0f, 0.0f, deltaTime);

	// Warp the bottom left game entity.
	static float scalar = 1.002f;
	Transform* transform = entities[1]->GetTransform();
	if (transform->GetScale().x > 1.5f || transform->GetScale().x < 0.66f)
		scalar = 2 - scalar;
	entities[1]->GetTransform()->Scale(scalar, scalar, 1.0f);

	// Move the three rightmost game entites slowly in a sinusoidal pattern.
	float speed = -0.5f * sinf(totalTime) * deltaTime;
	for (int i = 2; i < 5; i++) {
		entities[i]->GetTransform()->MoveAbsolute(speed, 0.0f, 0.0f);
	}
	*/

}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	// Draw each of the entities.
	for (int i = 0; i < entities.size(); i++) {

		// Set the vertex and pixel shaders to use for the next Draw() command
		entities[i]->GetMaterial()->GetVertexShader()->SetShader();
		entities[i]->GetMaterial()->GetPixelShader()->SetShader();

		// Create, map, and bind the constant buffer for the vertex shader.
		// - Create data for the constant buffer.
		// - Map the resource.
		std::shared_ptr<SimpleVertexShader> vs = entities[i]->GetMaterial()->GetVertexShader();
		vs->SetFloat4("colorTint", entities[i]->GetMaterial()->GetColorTint());
		vs->SetMatrix4x4("worldMatrix", entities[i]->GetTransform()->GetWorldMatrix());
		vs->SetMatrix4x4("viewMatrix", player->GetCamera()->GetViewMatrix());
		vs->SetMatrix4x4("projMatrix", player->GetCamera()->GetProjMatrix());
		vs->SetFloat4("specular", XMFLOAT4((float) entities[i]->GetMaterial()->GetSpecularExponent(), 0.0f, 0.0f, 0.0f));
		vs->CopyAllBufferData();

		// Set the pixel shader states required to display the texture.
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		ps->SetShaderResourceView("albedo", entities[i]->GetMaterial()->GetTextureSRVComPtr().Get());
		if (entities[i]->GetMaterial()->GetNormalMapSRVComPtr().Get() != nullptr)				// If the material has a normal map, set it.
			ps->SetShaderResourceView("normalMap", entities[i]->GetMaterial()->GetNormalMapSRVComPtr().Get());
		if (entities[i]->GetMaterial()->GetRoughnessSRVComPtr().Get() != nullptr) {				// If the material has PBR info, set it.
			ps->SetShaderResourceView("roughnessMap", entities[i]->GetMaterial()->GetRoughnessSRVComPtr().Get());
			ps->SetShaderResourceView("metalnessMap", entities[i]->GetMaterial()->GetMetalnessSRVComPtr().Get());
		}
		ps->SetSamplerState("samplerOptions", entities[i]->GetMaterial()->GetTextureSSComPtr().Get());

		// Set the data for the lights.
		ps->SetData("lights", (void*)(&lightShaderInputs[0]), sizeof(LightShaderInput) * MAX_LIGHTS);
		ps->SetFloat("numOfLights", lightShaderInputs.size());
		ps->SetData("cameraPos", &(player->GetCamera()->GetTransform().GetPosition()), sizeof(XMFLOAT3));
		ps->CopyBufferData("ExternalData");


		// Draw the entities
		DrawMesh(entities[i]->GetMesh());

		// Draw the sky.
		skybox->Draw(player->GetCamera(), context.Get());
	}

	


	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}


void Game::DrawMesh(Mesh* mesh)
{

	// Set buffers in the input assembler
	//  - Do this ONCE PER OBJECT you're drawing, since each object might
	//    have different geometry.
	//  - for this demo, this step *could* simply be done once during Init(),
	//    but I'm doing it here because it's often done multiple times per frame
	//    in a larger application/game
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, mesh->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);


	// Finally do the actual drawing
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER
	context->DrawIndexed(
		mesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}
