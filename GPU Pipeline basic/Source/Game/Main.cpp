#include "Common.h"
#include "Game.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE
	hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow) {
	// window 생성
	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	// gpu 초기화
	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
	// Destroy
	CleanupDevice();
	return (int)msg.wParam;
}