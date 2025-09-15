#include "Texture/Texture.h"

Texture::Texture(const std::filesystem::path& filePath)
    : m_filePath(filePath)
{ }

HRESULT Texture::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext)
{
    // 이미지 파일에서 텍스처 리소스 뷰 생성
    HRESULT hr = CreateWICTextureFromFile(
        pDevice,
        pImmediateContext,
        m_filePath.c_str(),
        nullptr,
        m_textureRV.GetAddressOf()
    );
    if (FAILED(hr)) {
        OutputDebugString(L"Can't load texture from file\n");
        OutputDebugString(m_filePath.c_str());
        return hr;
    }

    // 샘플러 상태 생성
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = pDevice->CreateSamplerState(&sampDesc, m_samplerLinear.GetAddressOf());
    if (FAILED(hr)) {
        OutputDebugString(L"Failed to create sampler state\n");
        return hr;
    }

    return S_OK;
}

ComPtr<ID3D11ShaderResourceView>& Texture::GetTextureResourceView() {
    return m_textureRV;
}

ComPtr<ID3D11SamplerState>& Texture::GetSamplerState() {
    return m_samplerLinear;
}
