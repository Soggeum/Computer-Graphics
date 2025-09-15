#pragma once

#include "Common.h"

#include "Shader/PixelShader.h"
#include "Shader/VertexShader.h"

#include "Renderer/DataTypes.h"
#include "Renderer/Material.h"
#include "Texture/DDSTextureLoader11.h"

static constexpr const UINT INVALID_MATERIAL = (0xFFFFFFFF);

struct BasicMeshEntry
{
	BasicMeshEntry()
		: uNumIndices(0u)
		, uBaseVertex(0u)
		, uBaseIndex(0u)
		, uMaterialIndex(INVALID_MATERIAL)
	{
	}

	UINT uNumIndices;      // sub-mesh�� �ε��� ����
	UINT uBaseVertex;      // ��ü ���ؽ� ���ۿ��� �� sub-mesh�� �����ϴ� ��ġ
	UINT uBaseIndex;       // ��ü �ε��� ���ۿ��� �� sub-mesh�� �����ϴ� ��ġ
	UINT uMaterialIndex;   // �� sub-mesh�� ������ ��Ƽ���� �ε���
};

class Renderable
{
protected:
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_cbChangeEveryFrame;

	std::vector<BasicMeshEntry> m_aMeshes;  // sub-mesh ������ �����ϴ� ���� ��� ���� ����
	std::vector<Material> m_aMaterials;     // material vector

	std::shared_ptr<VertexShader> m_vertexShader;
	std::shared_ptr<PixelShader> m_pixelShader;

	XMFLOAT4 m_outputColor;
	XMMATRIX m_world;

	const virtual SimpleVertex* getVertices() const = 0;
	virtual const WORD* getIndices() const = 0;

	HRESULT initialize(
		_In_ ID3D11Device* pDevice, 
		_In_ ID3D11DeviceContext* pImmediateContext
	);

	//// Texture class���� ó��
	//ComPtr<ID3D11ShaderResourceView> m_textureRV;
	//ComPtr<ID3D11SamplerState> m_samplerLinear;
	//std::filesystem::path m_textureFilePath;	

	//BOOL m_bHasTexture;

public:
	

	//Renderable(_In_ const std::filesystem::path& textureFilePath);
	Renderable(_In_ const XMFLOAT4& outputColor);
	Renderable(const Renderable& other) = delete;  // ���� ������ ����
	Renderable(Renderable&& other) = delete;
	Renderable() = default;
	virtual ~Renderable() = default;

	virtual HRESULT Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext) = 0;
	virtual void Update(_In_ FLOAT deltaTime) = 0;

	virtual UINT GetNumVertices() const = 0;
	virtual UINT GetNumIndices() const = 0;

	ComPtr<ID3D11VertexShader>& GetVertexShader();
	ComPtr<ID3D11PixelShader>& GetPixelShader();
	ComPtr<ID3D11InputLayout>& GetVertexLayout();

	ComPtr<ID3D11Buffer>& GetVertexBuffer();
	ComPtr<ID3D11Buffer>& GetIndexBuffer();
	ComPtr<ID3D11Buffer>& GetConstantBuffer();

	const XMMATRIX& GetWorldMatrix() const;
	const XMFLOAT4& GetOutputColor() const;
	BOOL HasTexture() const;

	// sub-mesh�� material ����
	const Material& GetMaterial(UINT uIndex) const;
	const BasicMeshEntry& GetMesh(UINT uIndex) const;

	void RotateX(_In_ FLOAT angle);
	void RotateY(_In_ FLOAT angle);
	void RotateZ(_In_ FLOAT angle);
	void RotateRollPitchYaw(_In_ FLOAT roll, _In_ FLOAT pitch, _In_ FLOAT yaw);
	void Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ);
	void Translate(_In_ const XMVECTOR& offset);

	void SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader);
	void SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader);

	UINT GetNumMeshes() const { return static_cast<UINT>(m_aMeshes.size()); }
	UINT GetNumMaterials() const { return static_cast<UINT>(m_aMaterials.size()); }

	// texture class ���� ó��
	//ComPtr<ID3D11ShaderResourceView>& GetTextureResourceView();
	//ComPtr<ID3D11SamplerState>& GetSamplerState();
};