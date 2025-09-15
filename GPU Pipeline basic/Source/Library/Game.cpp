#include "Game.h"

// assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

LPCWSTR g_pszWindowClassName = L"GGPWindowClass";
HWND g_hWnd = nullptr;

D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11Device1* g_pd3dDevice1 = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
IDXGISwapChain1* g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11PixelShader* g_pPixelSolidShader = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11InputLayout* g_pVertexLayout = nullptr;

// world, view, projection matrix�� ���� ����
// Matrix ����ü
ID3D11Buffer* g_pCBChangeOnCameraMovement = nullptr;
ID3D11Buffer* g_pCBProjection = nullptr;
ID3D11Buffer* g_pCBChangesEveryFrame = nullptr;
// Matrix
XMMATRIX g_worldMatrix;
XMMATRIX g_worldMatrix1;
XMMATRIX g_worldMatrix2;
XMFLOAT4 g_outColor;
XMFLOAT4 g_outColor1;
XMFLOAT4 g_outColor2;
XMMATRIX g_viewMatrix;
XMFLOAT4 g_cameraPosition;
XMMATRIX g_projectionMatrix;

// depth buffer
ID3D11Texture2D* g_pDepthStencil = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;

// texture mapping
ID3D11ShaderResourceView* g_pTextureRV = nullptr;  // texture image
ID3D11SamplerState* g_pSamplerLinear = nullptr;  // state

//lighting
ID3D11Buffer* g_pCBLights = nullptr;


HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
{
	// window class ����
	WNDCLASSEX wcex =
	{
	.cbSize = sizeof(WNDCLASSEX),
	.style = CS_HREDRAW | CS_VREDRAW,
	.lpfnWndProc = WindowProc,
	.cbClsExtra = 0,
	.cbWndExtra = 0,
	.hInstance = hInstance,
	.hIcon = LoadIcon(hInstance,IDI_APPLICATION),
	.hCursor = LoadCursor(nullptr, IDC_ARROW),
	.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
	.lpszMenuName = nullptr,
	.lpszClassName = g_pszWindowClassName,
	.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION),
	};

	// window class ���
	if (!RegisterClassEx(&wcex))
	{
		DWORD dwError = GetLastError();
		MessageBox(
			nullptr,
			L"Call to RegisterClassEx failed!",
			L"Game Graphics Programming",
			NULL
		);
		if (dwError != ERROR_CLASS_ALREADY_EXISTS)
		{
			return HRESULT_FROM_WIN32(dwError);
		}
		return E_FAIL;
	}

	/*HWND g_hWnd = nullptr;*/	// ����������
	g_hWnd = nullptr;

	HINSTANCE g_hInstance = nullptr;
	LPCWSTR g_pszWindowName = L"GGP02: Direct3D 11 Basics";
	g_hInstance = hInstance;

	// client â ũ�⸣�� �������� window ũ�� ����
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// window ����
	g_hWnd = CreateWindow(
		g_pszWindowClassName,
		g_pszWindowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (!g_hWnd)
	{
		DWORD dwError = GetLastError();
		MessageBox(
			nullptr,
			L"Call to CreateWindow failed!",
			L"Game Graphics Programming",
			NULL
		);
		if (dwError != ERROR_CLASS_ALREADY_EXISTS)
		{
			return HRESULT_FROM_WIN32(dwError);
		}
		return E_FAIL;
	}

	// ȭ�鿡 â ǥ��
	ShowWindow(g_hWnd, nCmdShow);
	return S_OK;
}

LRESULT CALLBACK WindowProc(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		if (MessageBox(hWnd,
			L"��¥ �����Ͻðڽ��ϱ�?",
			L"Game Graphics Programming",
			MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hWnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// DirectX �������� ���� WM_PAINT ó��
	case WM_PAINT:
		// TODO: ���⿡ ������ �ڵ� �߰�
		ValidateRect(hWnd, nullptr); // ��ȿȭ ���� ����
		return 0;

		// â ũ�� ���� �� ó�� (DirectX ���ҽ� �����)
	case WM_SIZE:
		break;

		// ��Ÿ �޽����� �⺻ ó��
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

HRESULT InitDevice() // resource ����
{
	HRESULT hr = S_OK;
	// client â ũ�� ��������
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// ����� ��ġ ã��
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_DRIVER_TYPE driverTypes[] =
	{
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] =
	{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels,
			numFeatureLevels, D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel,
			&g_pImmediateContext);
		if (hr == E_INVALIDARG)
		{
			// for DirectX 11.0
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags,
				&featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, &g_pd3dDevice,
				&g_featureLevel, &g_pImmediateContext);
		}
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	IDXGIFactory1* dxgiFactory = nullptr;
	IDXGIDevice* dxgiDevice = nullptr;
	hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice),
		reinterpret_cast<void**>(&dxgiDevice));
	if (SUCCEEDED(hr))
	{
		IDXGIAdapter* adapter = nullptr;
		hr = dxgiDevice->GetAdapter(&adapter);
		if (SUCCEEDED(hr))
		{
			hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
			adapter->Release();
		}
		dxgiDevice->Release();
	}
	if (FAILED(hr))
		return hr;

	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2),
		reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1),
			reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1),
				reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		// swap chain ���� ����
		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		// swap chain ������ �̿��� swap chain ����
		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr,
			nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain),
				reinterpret_cast<void**>(&g_pSwapChain));
		}
		dxgiFactory2->Release();
	}

	// back buffer �ּ� ��������
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;
	// backbuffer �ּ� ���� render target ����
	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
		&g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// render target ����������, �� target�� �°� depth buffer�� �ٷ� ����
	// depth buffer metadata ���� ����
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	// ������ metadata�� buffer ����
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr,
		&g_pDepthStencil);
	if (FAILED(hr))
		return hr;
	// DepthStencilView metadata ����
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	// metadata�� view resource ����
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV,
		&g_pDepthStencilView);
	if (FAILED(hr))
		return hr;
	// ���� Ÿ�� ��� ���� ���ٽ� �並 OM���������ο� ���ε�. backbuffer �ϼ�
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView,
		g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	// vertex buffer ����
	// user buffer�� vertex ����
	SimpleVertex sVertices[] =
	{
		// pos + UV + normal
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	};

	// cpu, gpu ������ vertex buffer�� metadata
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;  // �� flag ��� render���� UpdateSubresource ��밡��. D3D11_USAGE_DYNAMIC �̰� ���� �Ұ���
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// vertex buffer �����ϱ� ���� �ʱ�ȭ ����ü
	// buffer �����Ǹ鼭 �ٷ� GPU�� �����. 
	// �߰����� mapping���� �ʿ� ������
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = sVertices;
	// CreateBuffer�� GPU memory�� �Ҵ� �ϴ� �Լ�
	hr = g_pd3dDevice->CreateBuffer(&bd,  // bd�� description���� �޸� ����
		&initData,  // �ʱ�ȭ�� ������
		&g_pVertexBuffer); // �Ҵ�� buffer ��ü�� ������
	if (FAILED(hr))
		return hr;

	// index buffer
	WORD sIndices[] = {
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	// cpu, gpu ������ index buffer�� metadata 'bd' ����
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// �ʱ�ȭ ���� ����ü ���� 
	initData.pSysMem = sIndices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// �����ڷ�� input layout���� ������, ������ shader compile ���� ��
	// "VS.hlsl"�� �ε��ϰ� "VShader" �Լ��� ã�� 5.0 version���� ���ؽ� ���̴��� �������� ��, 
	// �����ϵ� ����� VS ��ӿ� ����
	// �׳� byte code�� ���常 �� ��
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* VS, * PS, * PSSolid;
	ID3DBlob* pErrorBlob = nullptr;

	CompileShaderFromFile(L"../../Source/Library/VS.hlsl", "VS", "vs_5_0", &VS);
	CompileShaderFromFile(L"../../Source/Library/PS.hlsl", "PS", "ps_5_0", &PS);
	CompileShaderFromFile(L"../../Source/Library/PSSolid.hlsl", "PSSolid", "ps_5_0", &PSSolid);


	// �� ���̴��� ���̴� ��ü�� ĸ��ȭ 
	// ������ byte code�� GPU�� ������ �� �ִ� native shader ��ü�� ��ȯ. GPU�� ��밡��(dev)
	hr = g_pd3dDevice->CreateVertexShader(
		VS->GetBufferPointer(),
		VS->GetBufferSize(),
		nullptr,
		&g_pVertexShader);
	if (FAILED(hr))
	{
		VS->Release();
		return hr;
	}

	hr = g_pd3dDevice->CreatePixelShader(
		PS->GetBufferPointer(),
		PS->GetBufferSize(),
		nullptr,
		&g_pPixelShader);
	if (FAILED(hr))
	{
		PS->Release();
		return hr;
	}

	hr = g_pd3dDevice->CreatePixelShader(
		PSSolid->GetBufferPointer(),
		PSSolid->GetBufferSize(),
		nullptr,
		&g_pPixelSolidShader);
	if (FAILED(hr))
	{
		PSSolid->Release();
		return hr;
	}

	// pipeline�� binding(devcon)�� render()���� ��
	/*devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);*/

	// �Է� ���̾ƿ� ��ü�� ����
	// Input layout : GPU���� vertex buffer�� ����� �������� ������ �˷��ִ� ��ü
	// ���� vertex buffer�� �̿��ؼ� ����
	//�Է� ��� �����
	D3D11_INPUT_ELEMENT_DESC layouts[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT uNumElements = ARRAYSIZE(layouts);

	// input layout ��ü ����(dev)
	hr = g_pd3dDevice->CreateInputLayout(
		layouts,
		uNumElements,
		VS->GetBufferPointer(),
		VS->GetBufferSize(),
		&g_pVertexLayout
	);
	VS->Release();
	if (FAILED(hr))
		return hr;
	// ���� ���� input layout�� input assembler�� binding.
	// �ٵ� binding�� render����
	// vertex buffer�� index buffer�� IA�� binding. render����

	// ���� Matirx
	// GPU�� constant buffer �޸𸮸� �Ҵ�(����ü)
	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	// world matrix
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangesEveryFrame);
	if (FAILED(hr)) return hr;
	// view matrix
	bd.ByteWidth = sizeof(CBChangeOnCameraMovement);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBChangeOnCameraMovement);
	if (FAILED(hr)) return hr;
	// Projection matrix
	bd.ByteWidth = sizeof(CBProjection);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCBProjection);
	if (FAILED(hr)) return hr;

	// Matrix�� cpu���� ������־�� ��
	g_worldMatrix = XMMatrixIdentity();  // world matrix �ʱ�ȭ
	g_worldMatrix1 = XMMatrixIdentity();
	g_worldMatrix2 = XMMatrixIdentity();
	g_outColor = { 0.0f, 0.0f, 0.0f, 1.0f };  // ��� cube. texture �����Ŷ� �� ��� ����
	g_outColor1 = { 1.0f, 1.0f, 1.0f, 1.0f };  // ��� ����
	g_outColor2 = { 0.5f, 0.0f, 0.0f, 1.0f };  // ������ ����
	XMVECTOR eye = XMVectorSet(-2.0f, 3.0f, -5.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_viewMatrix = XMMatrixLookAtLH(eye, at, up);
	g_cameraPosition = { 0.0f, 1.0f, -5.0f, 1.0f };
	g_projectionMatrix = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		width / (FLOAT)height,
		0.01f,
		100.0f
	);
	// matrix�� ����ü�� �ִ°Ͱ� gpu�� �ִ� �ñ�� ���������� render����(frame���� �ٲ�ϱ�)

	// ���� texture mapping �ܰ�. 
	// texture image ��ü ���� + texture SamplerState �ܰ� ���� 2�ܰ�� �̷����
	// texture image�� object�� ������. binding�� render����
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"../Library/seafloor.dds", nullptr, &g_pTextureRV);
	if (FAILED(hr))
		return hr;
	// 2�ܰ�:texture SamplerState description ����ü ����
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // ���� ���͸�
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // U�� ����
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // V�� ����
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    // W�� ����
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Desc�� SamplerState ����. binding�� Render����
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// lighting buffer ����
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBLights);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBLights);
	if (FAILED(hr))
		return hr;
	// �������� ������ render����

	// Assimp
	Assimp::Importer importer;  // importer ������Ʈ�� ���� �� ���� ����
	const aiScene* pScene =
		importer.ReadFile("../../Data/Tetrahedron.obj",
			aiProcess_Triangulate | aiProcess_MakeLeftHanded);  // importer���� readfile �Լ� ����ϰ�, obj������ �о� pScene�� ����
	// �� �Ǿ��� Ȯ��(debug)
	static CHAR szDebugMessage[256];
	sprintf_s(szDebugMessage, "Number of meshes found in file: % d\nNumber of verticies in first mesh : % d\n",
		pScene->mNumMeshes, pScene->mMeshes[0]->mNumVertices);
	OutputDebugStringA(szDebugMessage);

	return S_OK;
}


void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pPixelSolidShader) g_pPixelSolidShader->Release();
	if (g_pCBChangeOnCameraMovement) g_pCBChangeOnCameraMovement->Release();
	if (g_pCBProjection) g_pCBProjection->Release();
	if (g_pCBChangesEveryFrame) g_pCBChangesEveryFrame->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pTextureRV) g_pTextureRV->Release();
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pCBLights) g_pCBLights->Release();
}

void Render()	// init���� ������ resource�� GPU�� binding�ϴ� ����. drawȣ���ϸ� gpu pipeline ����
// �׷��Ƿ� draw�������� �ܼ� binding�̹Ƿ�, render �ȿ��� �ڵ� ��ġ�� ���� �����ϴ� ���谡 �ƴ϶�� ��� �ֵ� ��� ����
// ��κ��� ���� :	color/depth buffer �ʱ�ȭ (ClearRenderTargetView, ClearDepthStencilView)
//					������Ʈ �ʿ��� constant buffer(����, WVM matrix, texture...)�� update
//					Pipeline state(���̴�, ���� ��) ���� �� binding
//					Draw�� ������Ʈ ������
//					�ٽ� �ݺ��ϸ� object ������ ��, �������� present�� front/back buffer swap
{
	// �� frame���� color buffer, depth buffer �ʱ�ȭ
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
		D3D11_CLEAR_DEPTH,
		1.0f,  // �ʱ� ���̰�
		0);


	// ���� ����
	XMFLOAT4 vLightPositions[2] =
	{
	XMFLOAT4(-2.5f, 2.5f,-2.5f, 1.0f),
	XMFLOAT4(0.0f, 0.0f, -3.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[2] =
	{
	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f)
	};

	// frame �� matrix ���(rotation)
	static float t = 0.0f;
	if (g_driverType ==
		D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1000.0f;
	}
	XMMATRIX mRotate = XMMatrixRotationY(-2.0f * t);  // Y�� ȸ����Ű�� ���
	XMVECTOR vLightPos = XMLoadFloat4(&vLightPositions[1]);  // ��ġ�� ����ȭ
	vLightPos = XMVector3Transform(vLightPos, mRotate);  // ��ġ(vector)�� ȸ����� ����
	XMStoreFloat4(&vLightPositions[1], vLightPos);  // ���� ��ġ ��� ����

	CBLights cbLights;  // constant buffer�� �� ���� ��ü
	cbLights.LightPositions[0] = vLightPositions[0];
	cbLights.LightPositions[1] = vLightPositions[1];
	cbLights.LightColors[0] = vLightColors[0];
	cbLights.LightColors[1] = vLightColors[1];
	g_pImmediateContext->UpdateSubresource(g_pCBLights, 0, nullptr, &cbLights, 0, 0);
	g_pImmediateContext->PSSetConstantBuffers(3, 1, &g_pCBLights); // PS���� ����ϹǷ� binding. (b3)�� ���

	// object���� render
	// ��� cube ����
	g_worldMatrix = XMMatrixRotationY(t);
	// Matrix ����ü�� �� ����
	CBChangesEveryFrame cbCEF;
	CBChangeOnCameraMovement cbCOM;
	CBProjection cbProj;
	cbCEF.mWorld = XMMatrixTranspose(g_worldMatrix);
	cbCEF.OutColor = g_outColor;
	cbCOM.mView = XMMatrixTranspose(g_viewMatrix);
	cbCOM.CameraPosition = g_cameraPosition;
	cbProj.mProjection = XMMatrixTranspose(g_projectionMatrix);
	// ���� GPU memory�� �ִ� ��ü�� �� ����
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cbCEF, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pCBChangeOnCameraMovement, 0, nullptr, &cbCOM, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pCBProjection, 0, nullptr, &cbProj, 0, 0);
	// VS�� Matrix ��ü binding
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame); // register(b2)
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangeOnCameraMovement); // register(b0)
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBProjection); // register(b1)
	// cbChangeOnCameraMovement�� PS���� binding �ʿ�. slot ���� ���� ����ϵ��� �ؾ���
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangeOnCameraMovement); // register(b0)
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame); // register(b2)

	// ������ shader resource binding ������ ���⼭
	// render�� ������ �̿��� shader�� ������ �޶��� �� �����Ƿ�
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);  // texture image PS�� binding
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);  // SamplerState�� PS�� binding

	// input layout�� input assembler�� binding.
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// vertex buffer�� index buffer�� IA�� binding
	UINT uStride = sizeof(SimpleVertex);
	UINT uOffset = 0;
	g_pImmediateContext->IASetVertexBuffers(
		0, // slot
		1, // number of buffer
		&g_pVertexBuffer, // vertex buffer
		&uStride, // stride
		&uOffset // offset
	);
	g_pImmediateContext->IASetIndexBuffer(
		g_pIndexBuffer, // index buffer
		DXGI_FORMAT_R16_UINT, // format of the index data
		0 // offset
	);

	// primitive ����
	g_pImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	// ��ο� ȣ��. GPU pipeline ����
	g_pImmediateContext->DrawIndexed(36, 0, 0);


	// ���� object ������
	// �ٸ� object�� ������ �Ϸ��� �ٲ� �͵鸸 update ���ְ� �ٽ� draw ȣ���ϸ� �ȴ�
	CBChangesEveryFrame cbCEF1;	// ��ü���� world ��ĸ� �ٲ�. ������ ����� �� �ٲٰ� ����
	for (int m = 0; m < 2; m++)
	{
		XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&vLightPositions[m]));  // ������ ��ġ�� �̵���Ű�� ��� 
		mLight = XMMatrixScaling(0.2f, 0.2f, 0.2f) * mLight;  // ���� ũ�� ��ҽ�Ű��, �̵���Ű�� ���
		cbCEF1.mWorld = XMMatrixTranspose(mLight);
		cbCEF1.OutColor = vLightColors[m];
		// �ٲ� matrix ��ü�� update
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cbCEF1, 0, 0);
		// �̹� binding ������ ������ �̷�������Ƿ�, binding�� �͵��� update�� update�� ���ְ� �߰����� binding �ʿ� ����
		// ������ binding�ߴ� cbuffer��(View, Projection)�� Shader �ٲ� �״�� ���޵�

		// �����̶� PSSolid shader�� �ٲٹǷ� PSSolid shader�� �ٲپ��ָ� �ȴ�
		g_pImmediateContext->PSSetShader(g_pPixelSolidShader, nullptr, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}

	// backbuffer swap. ȭ�鿡 ���
	g_pSwapChain->Present(0, 0);
}

HRESULT CompileShaderFromFile(
	_In_ PCWSTR pszFileName,
	_In_ PCSTR pszEntryPoint,
	_In_ PCSTR pszShaderModel,
	_Outptr_ ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
	// ����� ���� ���� �� ����ȭ ��Ȱ��ȭ �ɼ�
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(
		pszFileName,
		nullptr,         // shader macros
		nullptr,         // include files
		pszEntryPoint,   // Entry point (�Լ� �̸�)
		pszShaderModel,  // shader target
		dwShaderFlags,   // flag1 
		0,               // flag2
		ppBlobOut,       // �����ϵ� ���̴� ����Ʈ�ڵ�
		&pErrorBlob      // ���� �޽���
	);

	if (FAILED(hr))
	{
		if (pErrorBlob) {
			OutputDebugStringA(
				reinterpret_cast<PCSTR>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();
	return S_OK;
}
