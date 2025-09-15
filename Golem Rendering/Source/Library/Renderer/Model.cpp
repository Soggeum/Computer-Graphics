#include "Renderer/Model.h"

Model::Model(_In_ const std::filesystem::path& filePath)
    : Renderable(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f))
    , m_filePath(filePath)
    , m_aVertices()
    , m_aIndices()
{
}

void Model::Update(float deltaTime) {}
UINT Model::GetNumVertices() const { return static_cast<UINT>(m_aVertices.size()); }
UINT Model::GetNumIndices() const { return static_cast<UINT>(m_aIndices.size()); }

const SimpleVertex* Model::getVertices() const { return m_aVertices.data(); }
const WORD* Model::getIndices() const { return m_aIndices.data(); }

HRESULT Model::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr = S_OK;
    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(  // ������ ȣ���� pScene�� ����
        m_filePath.string().c_str(),
        ASSIMP_LOAD_FLAGS | aiProcess_ConvertToLeftHanded
    );
    if (pScene)
    {
        hr = initFromScene(pDevice, pImmediateContext, pScene, m_filePath);  // �����ϸ� ȣ��
    }
    else
    {
        hr = E_FAIL;
        OutputDebugString(L"Error parsing ");
        OutputDebugString(m_filePath.c_str());
        OutputDebugString(L": ");
        OutputDebugStringA(importer.GetErrorString());
        OutputDebugString(L"\n");
    }
    return hr;
}

HRESULT Model::initFromScene(
    _In_ ID3D11Device* pDevice,
    _In_ ID3D11DeviceContext* pImmediateContext,
    _In_ const aiScene* pScene,
    _In_ const std::filesystem::path& filePath
)
{
    HRESULT hr = S_OK;

    m_aMeshes.resize(pScene->mNumMeshes);  // model�� mesh, material�� ����ŭ vector size ����
    m_aMaterials.resize(pScene->mNumMaterials);
    
    UINT uNumVertices = 0u;  
    UINT uNumIndices = 0u;

    countVerticesAndIndices(uNumVertices, uNumIndices, pScene);  // 3D model�� ��ü vertex, index ���� ����

    reserveSpace(uNumVertices, uNumIndices);  // �� ���� ���� vertice/index vector �޸� �Ҵ�

    initAllMeshes(pScene);  // mesh �ʱ�ȭ

    hr = initMaterials(pDevice, pImmediateContext, pScene, filePath);  // material �ʱ�ȭ
    if (FAILED(hr))
        return hr;

    hr = initialize(pDevice, pImmediateContext);  // vertex, index, cbuffer�� �ʱ�ȭ
    if (FAILED(hr))
        return hr;

    return hr;
}

void Model::countVerticesAndIndices(_Inout_ UINT& uOutNumVertices,
    _Inout_ UINT& uOutNumIndices, _In_ const aiScene* pScene)

{
    for (UINT i = 0u; i < m_aMeshes.size(); ++i)  // mesh���� ���� �� mesh�� �������� BasicMeshEntry�� ����
    {
        // ���� ����
        m_aMeshes[i].uMaterialIndex = pScene->mMeshes[i] -> mMaterialIndex;
        m_aMeshes[i].uNumIndices = pScene->mMeshes[i]->mNumFaces * 3u;  // �� �ϳ��� ���� 3���̹Ƿ�
        m_aMeshes[i].uBaseVertex = uOutNumVertices; 
        m_aMeshes[i].uBaseIndex = uOutNumIndices;

        // ���� �ջ�
        uOutNumVertices += pScene->mMeshes[i]->mNumVertices;
        uOutNumIndices += m_aMeshes[i].uNumIndices;
    }
}

void Model::reserveSpace(_In_ UINT uNumVertices, _In_ UINT uNumIndices)  // �޸� �Ҵ�
{
    m_aVertices.reserve(uNumVertices);
    m_aIndices.reserve(uNumIndices);
}

void Model::initAllMeshes(_In_ const aiScene* pScene)
{
    for (UINT i = 0u; i < m_aMeshes.size(); ++i)  // mesh ���� 
    {
        const aiMesh* pMesh = pScene->mMeshes[i];  // �ʱ�ȭ�ϰ�
        initSingleMesh(pMesh);  // mesh �ʱ�ȭ �Լ� ȣ��
    }
}

void Model::initSingleMesh(_In_ const aiMesh* pMesh)  // �� mesh���� �ʱ�ȭ
{
    const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
    // Populate the vertex attribute vectors
    for (UINT i = 0u; i < pMesh->mNumVertices; ++i)  // mesh�� ��� vertex ���� vertex ����(pos, uv coordinate, noraml)����
    {
        const aiVector3D& position = pMesh->mVertices[i];
        const aiVector3D& normal = pMesh->mNormals[i];
        const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ? pMesh->mTextureCoords[0][i] : zero3d;
        m_aVertices.push_back(
            SimpleVertex
            {
            .Position = XMFLOAT3(position.x, position.y, position.z),
            .TexCoord = XMFLOAT2(texCoord.x, texCoord.y),
            .Normal = XMFLOAT3(normal.x, normal.y, normal.z)
            }
        );
    }

    // Populate the index buffer
    for (UINT i = 0u; i < pMesh->mNumFaces; ++i)
    {
        const aiFace& face = pMesh->mFaces[i];
        assert(face.mNumIndices == 3u);
        m_aIndices.push_back(static_cast<WORD>(face.mIndices[0]));
        m_aIndices.push_back(static_cast<WORD>(face.mIndices[1]));
        m_aIndices.push_back(static_cast<WORD>(face.mIndices[2]));
    }
}

HRESULT Model::initMaterials(_In_ ID3D11Device* pDevice, _In_
    ID3D11DeviceContext* pImmediateContext, _In_ const aiScene* pScene, _In_
    const std::filesystem::path& filePath)
{
    HRESULT hr = S_OK;

    // Extract the directory part from the file name
    std::filesystem::path parentDirectory = filePath.parent_path();

    // Initialize the materials
    for (UINT i = 0u; i < pScene->mNumMaterials; ++i)  // material ���鼭 
    {
        const aiMaterial* pMaterial = pScene->mMaterials[i];
        loadTextures(pDevice, pImmediateContext, parentDirectory, pMaterial, i);  // texture(diffuse, specular) load
        loadColors(pMaterial, i);  // color load
    }
    return hr;
}

HRESULT Model::loadTextures(_In_ ID3D11Device* pDevice, _In_
    ID3D11DeviceContext* pImmediateContext, _In_ const std::filesystem::path&
    parentDirectory, _In_ const aiMaterial* pMaterial, _In_ UINT uIndex)
{
    HRESULT hr = loadDiffuseTexture(pDevice, pImmediateContext, parentDirectory, pMaterial, uIndex); // diffuse
    if (FAILED(hr))
        return hr;

    hr = loadSpecularTexture(pDevice, pImmediateContext, parentDirectory, pMaterial, uIndex);  // specular
    if (FAILED(hr))
        return hr;

    return hr;
}

HRESULT Model::loadDiffuseTexture(_In_ ID3D11Device* pDevice, 
    _In_ ID3D11DeviceContext* pImmediateContext, 
    _In_ const std::filesystem::path& parentDirectory, 
    _In_ const aiMaterial* pMaterial, 
    _In_ UINT uIndex)
{
    HRESULT hr = S_OK;
    m_aMaterials[uIndex].pDiffuse = nullptr;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString aiPath;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
        {
            std::string szPath(aiPath.data);
            if (szPath.substr(0ull, 2ull) == ".\\")
            {
                szPath = szPath.substr(2ull, szPath.size() - 2ull);
            }
            std::filesystem::path fullPath = parentDirectory / szPath;
            m_aMaterials[uIndex].pDiffuse = std::make_shared<Texture>(fullPath);
            hr = m_aMaterials[uIndex].pDiffuse->Initialize(pDevice,
                pImmediateContext);
            if (FAILED(hr))
            {
                OutputDebugString(L"Error loading diffuse texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
                return hr;
            }
            OutputDebugString(L"Loaded diffuse texture \"");
            OutputDebugString(fullPath.c_str());
            OutputDebugString(L"\"\n");
        }
    }
    return hr;
}

HRESULT Model::loadSpecularTexture(_In_ ID3D11Device* pDevice, 
    _In_ ID3D11DeviceContext* pImmediateContext, 
    _In_ const std::filesystem::path& parentDirectory, 
    _In_ const aiMaterial* pMaterial, 
    _In_ UINT uIndex)
{
    HRESULT hr = S_OK;
    m_aMaterials[uIndex].pSpecular = nullptr;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
    {
        aiString aiPath;
        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0u, &aiPath, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
        {
            std::string szPath(aiPath.data);
            if (szPath.substr(0ull, 2ull) == ".\\")
            {
                szPath = szPath.substr(2ull, szPath.size() - 2ull);
            }
            std::filesystem::path fullPath = parentDirectory / szPath;
            m_aMaterials[uIndex].pSpecular = std::make_shared<Texture>(fullPath);
            hr = m_aMaterials[uIndex].pSpecular -> Initialize(pDevice, pImmediateContext);
            if (FAILED(hr))
            {
                OutputDebugString(L"Error loading specular texture \"");
                OutputDebugString(fullPath.c_str());
                OutputDebugString(L"\"\n");
                return hr;
            }
            OutputDebugString(L"Loaded specular texture \"");
            OutputDebugString(fullPath.c_str());
            OutputDebugString(L"\"\n");
        }
    }
    return hr;
}

void Model::loadColors(_In_ const aiMaterial* pMaterial, _In_ UINT uIndex)
{
    // ambient �а� ����
    aiColor3D ambientColor(0.0f, 0.0f, 0.0f);  // �������� �ʱ�ȭ
    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) ==
        AI_SUCCESS)
    {
        m_aMaterials[uIndex].AmbientColor.x = ambientColor.r;
        m_aMaterials[uIndex].AmbientColor.y = ambientColor.g;
        m_aMaterials[uIndex].AmbientColor.z = ambientColor.b;
    }
    
    // diffuse
    aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) ==
        AI_SUCCESS)
    {
        m_aMaterials[uIndex].DiffuseColor.x = diffuseColor.r;
        m_aMaterials[uIndex].DiffuseColor.y = diffuseColor.g;
        m_aMaterials[uIndex].DiffuseColor.z = diffuseColor.b;
    }

    // specular
    aiColor3D specularColor(0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) ==
        AI_SUCCESS)
    {
        m_aMaterials[uIndex].SpecularColor.x = specularColor.r;
        m_aMaterials[uIndex].SpecularColor.y = specularColor.g;
        m_aMaterials[uIndex].SpecularColor.z = specularColor.b;
    }
}