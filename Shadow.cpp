#include "Shadow.h"
using namespace DirectX;


Shadow::Shadow(ID3D11Device* device, std::shared_ptr<SimpleVertexShader> vertexShader, int windowWidth, int windowHeight, int shadowMapSize)
{
	m_shadowMapSize = shadowMapSize;
	m_vertexShader = vertexShader;
	Shadow::OnWindowResize(windowWidth, windowHeight);

	// Texture of the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1; // Not an array
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // We'll bind as one or the other
	shadowDesc.CPUAccessFlags = 0; // Not reading back in C++
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS; // All 32 bits in a single channel (number)
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the texture itself
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Create the depth/stencil view for the shadow map
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0; // Render into the first mip
	device->CreateDepthStencilView(shadowTexture, &dsvDesc, m_dsv.GetAddressOf());

	// Create the shader resource view for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, m_srv.GetAddressOf());

	// Now that we have both views, we can release the
	// reference to the texture itself!
	shadowTexture->Release();

	// Create the rasterizer state to add bias to depth values
	// when creating the shadow map each frame
	D3D11_RASTERIZER_DESC shRast = {};
	shRast.FillMode = D3D11_FILL_SOLID;
	shRast.CullMode = D3D11_CULL_BACK;
	shRast.DepthClipEnable = true;
	shRast.DepthBias = 1000; // 1000 units of precision
	shRast.DepthBiasClamp = 0.0f;
	shRast.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shRast, m_rasterizer.GetAddressOf());


	// Create a sampler state for sampling the shadow map with
	// different options than we use for our "regular" textures
	D3D11_SAMPLER_DESC shSamp = {};
	shSamp.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shSamp.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shSamp.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shSamp.BorderColor[0] = 1.0f;
	shSamp.BorderColor[1] = 1.0f;
	shSamp.BorderColor[2] = 1.0f;
	shSamp.BorderColor[3] = 1.0f;
	shSamp.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // COMPARISON filter!  For use with comparison samplers!
	shSamp.ComparisonFunc = D3D11_COMPARISON_LESS;
	device->CreateSamplerState(&shSamp, m_sampler.GetAddressOf());


	// Create the view and projection for the shadow map
	XMMATRIX viewMatrix = XMMatrixLookToLH(
		XMVectorSet(0, 20, -20, 0), // "Backing up" along negative light dir
		XMVectorSet(0, -1, 1, 0), // This should always match the light's dir
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&m_viewMatrix, viewMatrix);

	XMMATRIX projMatrix = XMMatrixOrthographicLH(
		10,			// Width of projection "box" in world units
		10,			// Height of "
		0.1f,
		100.0f);
	XMStoreFloat4x4(&m_projMatrix, projMatrix);
}

void Shadow::Draw(std::vector<std::shared_ptr<GameEntity>> entities,
				  std::vector<Light*> lights,
				  ID3D11RenderTargetView** backBufferRTV, 
				  ID3D11DepthStencilView* depthStencilView, 
				  ID3D11DeviceContext* context)
{
	// Set the current render target and depth buffer
	// for shadow map creations
	// (Changing where the rendering goes!)
	context->OMSetRenderTargets(0, 0, m_dsv.Get()); // Only need the depth buffer (shadow map)
	context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Change any shadow-mapping-specific render states
	context->RSSetState(m_rasterizer.Get());

	// Create a viewport to match the new target size
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)m_shadowMapSize;
	vp.Height = (float)m_shadowMapSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// Set up vertex shader
	m_vertexShader->SetShader();
	context->PSSetShader(0, 0, 0); // Turns OFF the pixel shader!

	// Render each entity to each light.
	for (auto& light : lights) {

		if (light == nullptr) break;	// Failsafe, as size is returning capacity for some reason.

		// Get the view and proj matrix from the light.
		ViewAndProjMatrices vpMatrices = light->GetMatrices();
		m_vertexShader->SetMatrix4x4("viewMatrix", vpMatrices.View);
		m_vertexShader->SetMatrix4x4("projMatrix", vpMatrices.Proj);

		// Loop and render all entities
		for (auto& e : entities)
		{
			// Grab this entity's world matrix and
			// send to the VS
			m_vertexShader->SetMatrix4x4("worldMatrix", e->GetTransform()->GetWorldMatrix());
			m_vertexShader->CopyAllBufferData();

			// Only draw the current entity
			e->GetMesh()->Draw(context);
		}
	}


	// Reset anything I've changed
	context->OMSetRenderTargets(1, backBufferRTV, depthStencilView);
	vp.Width = m_width;
	vp.Height = m_height;
	context->RSSetViewports(1, &vp);
	context->RSSetState(0);
}

void Shadow::OnWindowResize(int width, int height)
{
	m_width = width;
	m_height = height;
}

DirectX::XMFLOAT4X4 Shadow::GetViewMatrix()
{
	return m_viewMatrix;
}

DirectX::XMFLOAT4X4 Shadow::GetProjMatrix()
{
	return m_projMatrix;
}
