#pragma once

#include "Common.h"
#include <d3d11.h>
#include <d3d11_4.h> 
#include <d3dcompiler.h> 
#include "DDSTextureLoader11.h"  // CreateDDSTextureFromFile

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler")

HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);

LRESULT CALLBACK WindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

HRESULT InitDevice();

struct SimpleVertex
{
	XMFLOAT3 Pos;
	//XMFLOAT4 Color;  // vertex color. 이제 색 대신 texture image로 사용
	XMFLOAT2 Material; // UV texture coordinates (u, v)
	XMFLOAT3 Normal;  // normal vector
};

// 각 Matrix를 따로 관리
struct CBChangeOnCameraMovement  // 기존 view Matrix + 카메라 위치 
{
	XMMATRIX mView;
	XMFLOAT4 CameraPosition;
};
struct CBProjection
{
	XMMATRIX mProjection;
};
struct CBChangesEveryFrame
{
	XMMATRIX mWorld;
	XMFLOAT4 OutColor;
};

// lighting
struct CBLights
{
	XMFLOAT4 LightPositions[2];   // 2개의 광원위치와
	XMFLOAT4 LightColors[2];      // 2개의 광원 색깔
};

void CleanupDevice();
void Render();

HRESULT CompileShaderFromFile(
	_In_ PCWSTR pszFileName, // FileName
	_In_ PCSTR pszEntryPoint, // EntryPoint
	_In_ PCSTR pszShaderModel, // Shader target
	_Outptr_ ID3DBlob** ppBlobOut // ID3DBlob out
);
