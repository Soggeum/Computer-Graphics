#pragma once
#include "Renderer/Renderable.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


// Assimp ���� ���� (������� include �ּ�ȭ)
struct aiScene;
struct aiMesh;
struct aiMaterial;

class Model : public Renderable
{
public:
    Model(_In_ const std::filesystem::path& filePath);
    Model() = default;
    virtual ~Model() = default;

    // Renderable�� ���� ���� �Լ� ����
    HRESULT Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext) override;
    void Update(FLOAT deltaTime) override;
    UINT GetNumVertices() const override;
    UINT GetNumIndices() const override;

protected:
    std::filesystem::path m_filePath;
    std::vector<SimpleVertex> m_aVertices;
    std::vector<WORD> m_aIndices;

    const SimpleVertex* getVertices() const override;
    const WORD* getIndices() const override;

    // --- Assimp �� �� �ε� ���� �Լ��� ---
    void countVerticesAndIndices(_Inout_ UINT& uOutNumVertices, _Inout_ UINT& uOutNumIndices, _In_ const aiScene* pScene);
    void initAllMeshes(_In_ const aiScene* pScene);
    void initSingleMesh(_In_ const aiMesh* pMesh);
    HRESULT initMaterials(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext, _In_ const aiScene* pScene, _In_ const std::filesystem::path& filePath);
    HRESULT initFromScene(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext, _In_ const aiScene* pScene, _In_ const std::filesystem::path& filePath    );

    void loadColors(_In_ const aiMaterial* pMaterial, _In_ UINT uIndex);
    HRESULT loadDiffuseTexture(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext, _In_ const std::filesystem::path& parentDirectory, _In_ const aiMaterial* pMaterial, _In_ UINT uIndex);
    HRESULT loadSpecularTexture(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext, _In_ const std::filesystem::path& parentDirectory, _In_ const aiMaterial* pMaterial, _In_ UINT uIndex);
    HRESULT loadTextures(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext, _In_ const std::filesystem::path& parentDirectory, _In_ const aiMaterial* pMaterial, _In_ UINT uIndex);

    void reserveSpace(_In_ UINT uNumVertices, _In_ UINT uNumIndices);
};
