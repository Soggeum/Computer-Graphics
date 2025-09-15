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

// world, view, projection matrix을 따로 관리
// Matrix 구조체
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
	// window class 정의
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

	// window class 등록
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

	/*HWND g_hWnd = nullptr;*/	// 전역변수로
	g_hWnd = nullptr;

	HINSTANCE g_hInstance = nullptr;
	LPCWSTR g_pszWindowName = L"GGP02: Direct3D 11 Basics";
	g_hInstance = hInstance;

	// client 창 크기르르 기준으로 window 크기 설정
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// window 생성
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

	// 화면에 창 표시
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
			L"진짜 종료하시겠습니까?",
			L"Game Graphics Programming",
			MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hWnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// DirectX 렌더링을 위해 WM_PAINT 처리
	case WM_PAINT:
		// TODO: 여기에 렌더링 코드 추가
		ValidateRect(hWnd, nullptr); // 무효화 영역 제거
		return 0;

		// 창 크기 변경 시 처리 (DirectX 리소스 재생성)
	case WM_SIZE:
		break;

		// 기타 메시지는 기본 처리
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

HRESULT InitDevice() // resource 생성
{
	HRESULT hr = S_OK;
	// client 창 크기 가져오기
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// 연결된 장치 찾기
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

		// swap chain 정보 저장
		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		// swap chain 정보를 이용해 swap chain 생성
		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr,
			nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain),
				reinterpret_cast<void**>(&g_pSwapChain));
		}
		dxgiFactory2->Release();
	}

	// back buffer 주소 가져오기
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;
	// backbuffer 주소 통해 render target 생성
	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
		&g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// render target 생성했으니, 이 target에 맞게 depth buffer도 바로 설정
	// depth buffer metadata 먼저 저장
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
	// 저장한 metadata로 buffer 생성
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr,
		&g_pDepthStencil);
	if (FAILED(hr))
		return hr;
	// DepthStencilView metadata 저장
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	// metadata로 view resource 생성
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV,
		&g_pDepthStencilView);
	if (FAILED(hr))
		return hr;
	// 렌더 타겟 뷰와 깊이 스텐실 뷰를 OM파이프라인에 바인딩. backbuffer 완성
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

	// vertex buffer 생성
	// user buffer에 vertex 생성
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

	// cpu, gpu 공유할 vertex buffer의 metadata
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;  // 이 flag 써야 render에서 UpdateSubresource 사용가능. D3D11_USAGE_DYNAMIC 이거 쓰면 불가능
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// vertex buffer 생성하기 위한 초기화 구조체
	// buffer 생성되면서 바로 GPU에 복사됨. 
	// 추가적인 mapping과정 필요 없어짐
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = sVertices;
	// CreateBuffer는 GPU memory에 할당 하는 함수
	hr = g_pd3dDevice->CreateBuffer(&bd,  // bd의 description따라 메모리 생성
		&initData,  // 초기화할 데이터
		&g_pVertexBuffer); // 할당된 buffer 객체의 포인터
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

	// cpu, gpu 공유할 index buffer의 metadata 'bd' 재사용
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// 초기화 위한 구조체 재사용 
	initData.pSysMem = sIndices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// 강의자료는 input layout으로 가지만, 실제론 shader compile 먼저 함
	// "VS.hlsl"을 로드하고 "VShader" 함수를 찾아 5.0 version으로 버텍스 셰이더로 컴파일한 후, 
	// 컴파일된 결과를 VS 블롭에 저장
	// 그냥 byte code로 저장만 한 것
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


	// 두 셰이더를 셰이더 객체로 캡슐화 
	// 저장한 byte code를 GPU가 이해할 수 있는 native shader 객체로 변환. GPU가 사용가능(dev)
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

	// pipeline에 binding(devcon)은 render()에서 함
	/*devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);*/

	// 입력 레이아웃 객체를 생성
	// Input layout : GPU에게 vertex buffer에 저장된 데이터의 구조를 알려주는 객체
	// 따라서 vertex buffer만 이용해서 생성
	//입력 요소 만들기
	D3D11_INPUT_ELEMENT_DESC layouts[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT uNumElements = ARRAYSIZE(layouts);

	// input layout 객체 생성(dev)
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
	// 이제 만든 input layout을 input assembler에 binding.
	// 근데 binding은 render에서
	// vertex buffer와 index buffer도 IA에 binding. render에서

	// 이젠 Matirx
	// GPU에 constant buffer 메모리를 할당(구조체)
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

	// Matrix는 cpu에서 계산해주어야 함
	g_worldMatrix = XMMatrixIdentity();  // world matrix 초기화
	g_worldMatrix1 = XMMatrixIdentity();
	g_worldMatrix2 = XMMatrixIdentity();
	g_outColor = { 0.0f, 0.0f, 0.0f, 1.0f };  // 가운데 cube. texture 입힐거라 색 상관 없음
	g_outColor1 = { 1.0f, 1.0f, 1.0f, 1.0f };  // 흰색 광원
	g_outColor2 = { 0.5f, 0.0f, 0.0f, 1.0f };  // 빨간색 광원
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
	// matrix를 구조체에 넣는것과 gpu에 넣는 시기는 마찬가지로 render에서(frame마다 바뀌니까)

	// 이제 texture mapping 단계. 
	// texture image 객체 생성 + texture SamplerState 단계 생성 2단계로 이루어짐
	// texture image를 object에 컴파일. binding은 render에서
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"../Library/seafloor.dds", nullptr, &g_pTextureRV);
	if (FAILED(hr))
		return hr;
	// 2단계:texture SamplerState description 구조체 설정
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 선형 필터링
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // U축 랩핑
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // V축 랩핑
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    // W축 랩핑
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Desc로 SamplerState 생성. binding은 Render에서
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	// lighting buffer 생성
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBLights);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCBLights);
	if (FAILED(hr))
		return hr;
	// 광원들의 설정은 render에서

	// Assimp
	Assimp::Importer importer;  // importer 오브젝트를 통해 모델 파일 관리
	const aiScene* pScene =
		importer.ReadFile("../../Data/Tetrahedron.obj",
			aiProcess_Triangulate | aiProcess_MakeLeftHanded);  // importer통해 readfile 함수 사용하고, obj파일을 읽어 pScene에 저장
	// 잘 되었나 확인(debug)
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

void Render()	// init으로 생성한 resource를 GPU에 binding하는 과정. draw호출하면 gpu pipeline 실행
// 그러므로 draw전까지는 단순 binding이므로, render 안에서 코드 위치는 서로 의존하는 관계가 아니라면 어디에 있든 상관 없다
// 대부분의 순서 :	color/depth buffer 초기화 (ClearRenderTargetView, ClearDepthStencilView)
//					업데이트 필요한 constant buffer(광원, WVM matrix, texture...)들 update
//					Pipeline state(셰이더, 버퍼 등) 설정 및 binding
//					Draw로 오브젝트 렌더링
//					다시 반복하며 object 렌더링 후, 마지막에 present로 front/back buffer swap
{
	// 매 frame마다 color buffer, depth buffer 초기화
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
		D3D11_CLEAR_DEPTH,
		1.0f,  // 초기 깊이값
		0);


	// 광원 설정
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

	// frame 별 matrix 계산(rotation)
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
	XMMATRIX mRotate = XMMatrixRotationY(-2.0f * t);  // Y축 회전시키는 행렬
	XMVECTOR vLightPos = XMLoadFloat4(&vLightPositions[1]);  // 위치를 벡터화
	vLightPos = XMVector3Transform(vLightPos, mRotate);  // 위치(vector)에 회전행렬 적용
	XMStoreFloat4(&vLightPositions[1], vLightPos);  // 최종 위치 결과 저장

	CBLights cbLights;  // constant buffer에 들어갈 광원 객체
	cbLights.LightPositions[0] = vLightPositions[0];
	cbLights.LightPositions[1] = vLightPositions[1];
	cbLights.LightColors[0] = vLightColors[0];
	cbLights.LightColors[1] = vLightColors[1];
	g_pImmediateContext->UpdateSubresource(g_pCBLights, 0, nullptr, &cbLights, 0, 0);
	g_pImmediateContext->PSSetConstantBuffers(3, 1, &g_pCBLights); // PS에서 사용하므로 binding. (b3)에 등록

	// object마다 render
	// 가운데 cube 먼저
	g_worldMatrix = XMMatrixRotationY(t);
	// Matrix 구조체에 값 대입
	CBChangesEveryFrame cbCEF;
	CBChangeOnCameraMovement cbCOM;
	CBProjection cbProj;
	cbCEF.mWorld = XMMatrixTranspose(g_worldMatrix);
	cbCEF.OutColor = g_outColor;
	cbCOM.mView = XMMatrixTranspose(g_viewMatrix);
	cbCOM.CameraPosition = g_cameraPosition;
	cbProj.mProjection = XMMatrixTranspose(g_projectionMatrix);
	// 이제 GPU memory에 있는 객체에 값 복사
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cbCEF, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pCBChangeOnCameraMovement, 0, nullptr, &cbCOM, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pCBProjection, 0, nullptr, &cbProj, 0, 0);
	// VS에 Matrix 객체 binding
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame); // register(b2)
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBChangeOnCameraMovement); // register(b0)
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCBProjection); // register(b1)
	// cbChangeOnCameraMovement은 PS에도 binding 필요. slot 같은 곳에 등록하도록 해야함
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBChangeOnCameraMovement); // register(b0)
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBChangesEveryFrame); // register(b2)

	// 생성한 shader resource binding 과정도 여기서
	// render할 때마다 이용할 shader의 종류가 달라질 수 있으므로
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);  // texture image PS에 binding
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);  // SamplerState를 PS에 binding

	// input layout을 input assembler에 binding.
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// vertex buffer와 index buffer도 IA에 binding
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

	// primitive 선택
	g_pImmediateContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	// 드로우 호출. GPU pipeline 실행
	g_pImmediateContext->DrawIndexed(36, 0, 0);


	// 광원 object 렌더링
	// 다른 object도 렌더링 하려면 바뀐 것들만 update 해주고 다시 draw 호출하면 된다
	CBChangesEveryFrame cbCEF1;	// 물체마다 world 행렬만 바뀜. 나머지 행렬은 안 바꾸고 재사용
	for (int m = 0; m < 2; m++)
	{
		XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&vLightPositions[m]));  // 광원의 위치로 이동시키는 행렬 
		mLight = XMMatrixScaling(0.2f, 0.2f, 0.2f) * mLight;  // 먼저 크기 축소시키고, 이동시키는 행렬
		cbCEF1.mWorld = XMMatrixTranspose(mLight);
		cbCEF1.OutColor = vLightColors[m];
		// 바뀐 matrix 객체만 update
		g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, nullptr, &cbCEF1, 0, 0);
		// 이미 binding 과정은 이전에 이루어졌으므로, binding된 것들의 update는 update만 해주고 추가적인 binding 필요 없음
		// 이전에 binding했던 cbuffer들(View, Projection)은 Shader 바뀌어도 그대로 전달됨

		// 광원이라 PSSolid shader로 바꾸므로 PSSolid shader만 바꾸어주면 된다
		g_pImmediateContext->PSSetShader(g_pPixelSolidShader, nullptr, 0);
		g_pImmediateContext->DrawIndexed(36, 0, 0);
	}

	// backbuffer swap. 화면에 출력
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
	// 디버그 정보 포함 및 최적화 비활성화 옵션
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(
		pszFileName,
		nullptr,         // shader macros
		nullptr,         // include files
		pszEntryPoint,   // Entry point (함수 이름)
		pszShaderModel,  // shader target
		dwShaderFlags,   // flag1 
		0,               // flag2
		ppBlobOut,       // 컴파일된 셰이더 바이트코드
		&pErrorBlob      // 에러 메시지
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
