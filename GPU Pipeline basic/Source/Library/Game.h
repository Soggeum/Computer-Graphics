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
	//XMFLOAT4 Color;  // vertex color. ���� �� ��� texture image�� ���
	XMFLOAT2 Material; // UV texture coordinates (u, v)
	XMFLOAT3 Normal;  // normal vector
};

// �� Matrix�� ���� ����
struct CBChangeOnCameraMovement  // ���� view Matrix + ī�޶� ��ġ 
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
	XMFLOAT4 LightPositions[2];   // 2���� ������ġ��
	XMFLOAT4 LightColors[2];      // 2���� ���� ����
};

void CleanupDevice();
void Render();

HRESULT CompileShaderFromFile(
	_In_ PCWSTR pszFileName, // FileName
	_In_ PCSTR pszEntryPoint, // EntryPoint
	_In_ PCSTR pszShaderModel, // Shader target
	_Outptr_ ID3DBlob** ppBlobOut // ID3DBlob out
);
