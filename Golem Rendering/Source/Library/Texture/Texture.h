#pragma once

#include "Common.h"

#include "Shader/PixelShader.h"
#include "Shader/VertexShader.h"

#include "Renderer/DataTypes.h"


#include "Texture/Texture.h"
#include "Texture/DDSTextureLoader11.h"

#include "Texture/WICTextureLoader.h"

class Texture {
public:
	Texture(const std::filesystem::path& filePath);
	~Texture() = default;

	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext);

	ComPtr<ID3D11ShaderResourceView>& GetTextureResourceView();
	ComPtr<ID3D11SamplerState>& GetSamplerState();

private:
	std::filesystem::path m_filePath;
	ComPtr<ID3D11ShaderResourceView> m_textureRV;
	ComPtr<ID3D11SamplerState> m_samplerLinear;
};