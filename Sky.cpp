
#include "Sky.h"
using namespace DirectX;

Sky::Sky(std::shared_ptr<Mesh> mesh,
	ID3D11ShaderResourceView* cubeMap,
	std::shared_ptr<SimpleVertexShader> vertexShader,
	std::shared_ptr<SimplePixelShader> pixelShader,
	ID3D11SamplerState* samplerOptions,
	ID3D11Device* device)
{
	// Instantiate variables
	skyGeometry = mesh;
	this->cubeMap = cubeMap;
	skyVertexShader = vertexShader;
	skyPixelShader = pixelShader;
	this->samplerOptions = samplerOptions;

	// Create render states
	// - Rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rasterizerDesc, rasterizerOptions.GetAddressOf());

	// - Depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthDesc, depthStencilState.GetAddressOf());
}


void Sky::Draw(Camera* camera, ID3D11DeviceContext* context)
{
	// Set rasterizer and depth state
	context->RSSetState(rasterizerOptions.Get());					
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);	

	// Activate the shaders.
	skyVertexShader->SetShader();
	skyPixelShader->SetShader();

	// Set registers
	skyPixelShader->SetShaderResourceView("cubeMap", cubeMap.Get());
	skyPixelShader->SetSamplerState("samplerOptions", samplerOptions.Get());

	// Give information to the vertex shader.
	skyVertexShader->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	skyVertexShader->SetMatrix4x4("projMatrix", camera->GetProjMatrix());
	skyVertexShader->CopyAllBufferData();

	// Draw the mesh
	DrawMesh(skyGeometry.get(), context);

	// Reset rasterizer and depth state
	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(nullptr, 0);
}


void Sky::DrawMesh(Mesh* mesh, ID3D11DeviceContext* context)
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