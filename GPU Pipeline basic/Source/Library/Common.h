#pragma once

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_MakeLeftHanded | aiProcess_JoinIdenticalVertices )

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"dxguid.lib")

#include <DirectXColors.h>
#include <DirectXMath.h>
using namespace DirectX;