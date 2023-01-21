// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <detours.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <Helpers.h>
#include <SigScan.h>

// d3d11
#include <d3d11.h>
#include <dxgi.h>

// imgui
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView = NULL;
WNDPROC oWndProc;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;
	if (ImGui::GetCurrentContext() != 0 && ImGui::GetIO().WantCaptureMouse) {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
			return true;
		}
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

typedef enum GameButton : uint32_t {
	UP       = 0x00000008,
	DOWN     = 0x00000010,
	LEFT     = 0x00000020,
	RIGHT    = 0x00000040,
	SQUARE   = 0x00000080,
	TRIANGLE = 0x00000100,
	CIRCLE   = 0x00000200,
	CROSS    = 0x00000400,
	L1       = 0x00000800,
	R1       = 0x00001000,
	L2       = 0x00002000,
	R2       = 0x00004000,
	L3       = 0x00008000,
	R3       = 0x00010000,
	TOUCHPAD = 0x00020000,
	START    = 0x01000000,
	SELECT   = 0x40000000
} GameButton;

typedef enum GameStick : uint32_t {
	LSTICK_UP    = 0x01000000,
	LSTICK_DOWN  = 0x02000000,
	LSTICK_LEFT  = 0x04000000,
	LSTICK_RIGHT = 0x08000000,
	RSTICK_UP    = 0x10000000,
	RSTICK_DOWN  = 0x20000000,
	RSTICK_LEFT  = 0x40000000,
	RSTICK_RIGHT = 0x80000000
} GameStick;

FUNCTION_PTR(void*, __stdcall, DivaGetInputState, 0x1402AC960, int32_t a1);

// Setups

short dispWidth;
short dispHeight;

ImColor MaruCol = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
ImColor BatsuCol = ImColor(0.0f, 0.0f, 1.0f, 1.0f);
ImColor SankakuCol = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
ImColor ShikakuCol = ImColor(1.0f, 0.5f, 0.5f, 1.0f);
ImColor SlideCol = ImColor(1.0f, 1.0f, 0.0f, 1.0f);
ImColor ShoulderCol = ImColor(0.5f, 0.5f, 0.5f, 1.0f);


extern "C" __declspec(dllexport) void Init()
{
}

extern "C" __declspec(dllexport) void D3DInit(IDXGISwapChain * swapChain, ID3D11Device * device, ID3D11DeviceContext * deviceContext) {
	pContext = deviceContext;

	DXGI_SWAP_CHAIN_DESC sd;
	swapChain->GetDesc(&sd);
	ID3D11Texture2D* pBackBuffer;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer);
	device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
	pBackBuffer->Release();
	HWND window = sd.OutputWindow;
	oWndProc = (WNDPROC)SetWindowLongPtrA(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, pContext);
}

extern "C" __declspec(dllexport) void onFrame(IDXGISwapChain * chain) {
	uint32_t btnState = *(uint32_t*)((char*)DivaGetInputState(0) + 0x30);
	uint32_t stickState = *(uint32_t*)((char*)DivaGetInputState(0) + 0x40);
	uint32_t startState = *(uint32_t*)((char*)DivaGetInputState(0) + 0x44);
	// check the window size

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImVec2 dispSize = ImGui::GetWindowSize();
	ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("InputDisplay", 0, ImGuiWindowFlags_NoTitleBar);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	// Button drawing, probably bad

	if (btnState & UP)
		draw_list->AddRectFilled(ImVec2(50, 75), ImVec2(75, 100), SankakuCol);
	else
		draw_list->AddRect(ImVec2(50, 75), ImVec2(75, 100), SankakuCol);

	if (btnState & TRIANGLE)
		draw_list->AddRectFilled(ImVec2(125, 75), ImVec2(150, 100), SankakuCol);
	else
		draw_list->AddRect(ImVec2(125, 75), ImVec2(150, 100), SankakuCol);

	if (btnState & DOWN)
		draw_list->AddRectFilled(ImVec2(50, 100), ImVec2(75, 125), BatsuCol);
	else
		draw_list->AddRect(ImVec2(50, 100), ImVec2(75, 125), BatsuCol);

	if (btnState & CROSS)
		draw_list->AddRectFilled(ImVec2(125, 100), ImVec2(150, 125), BatsuCol);
	else
		draw_list->AddRect(ImVec2(125, 100), ImVec2(150, 125), BatsuCol);

	if (btnState & LEFT)
		draw_list->AddRectFilled(ImVec2(25, 100), ImVec2(50, 125), ShikakuCol);
	else
		draw_list->AddRect(ImVec2(25, 100), ImVec2(50, 125), ShikakuCol);

	if (btnState & SQUARE)
		draw_list->AddRectFilled(ImVec2(100, 100), ImVec2(125, 125), ShikakuCol);
	else
		draw_list->AddRect(ImVec2(100, 100), ImVec2(125, 125), ShikakuCol);

	if (btnState & RIGHT)
		draw_list->AddRectFilled(ImVec2(75, 100), ImVec2(100, 125), MaruCol);
	else
		draw_list->AddRect(ImVec2(75, 100), ImVec2(100, 125), MaruCol);

	if (btnState & CIRCLE)
		draw_list->AddRectFilled(ImVec2(150, 100), ImVec2(175, 125), MaruCol);
	else
		draw_list->AddRect(ImVec2(150, 100), ImVec2(175, 125), MaruCol);

	if (btnState & L1)
		draw_list->AddRectFilled(ImVec2(25, 87.5), ImVec2(50, 100), ShoulderCol);
	else
		draw_list->AddRect(ImVec2(25, 87.5), ImVec2(50, 100), ShoulderCol);

	if (btnState & R1)
		draw_list->AddRectFilled(ImVec2(150, 87.5), ImVec2(175, 100), ShoulderCol);
	else
		draw_list->AddRect(ImVec2(150, 87.5), ImVec2(175, 100), ShoulderCol);

	if (btnState & L2)
		draw_list->AddRectFilled(ImVec2(25, 62.5), ImVec2(50, 87.5), ShoulderCol);
	else
		draw_list->AddRect(ImVec2(25, 62.5), ImVec2(50, 87.5), ShoulderCol);

	if (btnState & R2)
		draw_list->AddRectFilled(ImVec2(150, 62.5), ImVec2(175, 87.5), ShoulderCol);
	else
		draw_list->AddRect(ImVec2(150, 62.5), ImVec2(175, 87.5), ShoulderCol);

	// Slides

	if (stickState & LSTICK_LEFT)
		draw_list->AddRectFilled(ImVec2(25, 125), ImVec2(50, 137.5), SlideCol);
	else
		draw_list->AddRect(ImVec2(25, 125), ImVec2(50, 137.5), SlideCol);

	if (stickState & LSTICK_RIGHT)
		draw_list->AddRectFilled(ImVec2(75, 125), ImVec2(100, 137.5), SlideCol);
	else
		draw_list->AddRect(ImVec2(75, 125), ImVec2(100, 137.5), SlideCol);

	if (stickState & RSTICK_LEFT)
		draw_list->AddRectFilled(ImVec2(100, 125), ImVec2(125, 137.5), SlideCol);
	else
		draw_list->AddRect(ImVec2(100, 125), ImVec2(125, 137.5), SlideCol);

	if (stickState & RSTICK_RIGHT)
		draw_list->AddRectFilled(ImVec2(150, 125), ImVec2(175, 137.5), SlideCol);
	else
		draw_list->AddRect(ImVec2(150, 125), ImVec2(175, 137.5), SlideCol);

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	return TRUE;
}

