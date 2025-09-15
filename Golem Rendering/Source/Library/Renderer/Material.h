#pragma once
#include "Common.h"
#include "Texture/Texture.h"

#include <DirectXMath.h>

using namespace DirectX;

class Material {
public:
    // 텍스처
    std::shared_ptr<Texture> pDiffuse;
    std::shared_ptr<Texture> pSpecular;

    // 색상 정보
    XMFLOAT3 AmbientColor;
    XMFLOAT3 DiffuseColor;
    XMFLOAT3 SpecularColor;

    // 생성자
    Material()
        : pDiffuse(nullptr)
        , pSpecular(nullptr)
        , AmbientColor(0.0f, 0.0f, 0.0f)
        , DiffuseColor(1.0f, 1.0f, 1.0f)
        , SpecularColor(1.0f, 1.0f, 1.0f)
    {
    }

    ~Material() = default;
};
