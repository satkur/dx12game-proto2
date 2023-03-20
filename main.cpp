#include "main.h"

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;

// @brief コンソール画面に文字列を表示する
// @remarks デバッグ用
void debugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
    // va_list : 可変個の実引数を扱うための情報を保持するための型
    va_list valist;
    va_start(valist, format);
    vprintf(format, valist);
    va_end(valist);
#endif
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

ID3D12Device* _pDevice = nullptr;
IDXGIFactory6* _pDXGIFactory = nullptr;
IDXGISwapChain4* _pDXGISwapChain = nullptr;

LRESULT windowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_DESTROY) {
        // on window destroyed
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
    // create & register window class
    WNDCLASSEX w = {};
    w.cbSize = sizeof(WNDCLASSEX);
    w.lpfnWndProc = (WNDPROC)windowProcedure;
    w.lpszClassName = _T("dx12game-proto2");
    w.hInstance = GetModuleHandle(nullptr);
    RegisterClassEx(&w);

    RECT wRect = { 0, 0, window_width, window_height };
    AdjustWindowRect(&wRect, WS_OVERLAPPEDWINDOW, false);

    // create window object
    HWND hwnd = CreateWindow(
        w.lpszClassName,
        _T("DX12GAME-PROTO2"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wRect.right - wRect.left,
        wRect.bottom - wRect.top,
        nullptr,
        nullptr,
        w.hInstance,
        nullptr
    );

#pragma region DirectX12 initialization

    // create dxgi factory
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_pDXGIFactory));
    
    // enumerate adapters by dxgi factory
    std::vector<IDXGIAdapter*> adapters;
    IDXGIAdapter* adapterCache = nullptr;
    for (int i = 0;
         _pDXGIFactory->EnumAdapters(i, &adapterCache) != DXGI_ERROR_NOT_FOUND;
         i++
    ) {
        adapters.push_back(adapterCache);
    }

    // find desired adapter
    auto targetAdapterName = L"NVIDIA";
    for (auto adapter : adapters) {
        DXGI_ADAPTER_DESC desc = {};
        adapter->GetDesc(&desc); // get adapter desc

        std::wstring adapterStrDesc = desc.Description; // Description : adapter name

        if (adapterStrDesc.find(targetAdapterName) != std::string::npos) {
            adapterCache = adapter;
            break;
        }
    }

    
    // feature levels
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    // init device
    D3D_FEATURE_LEVEL featureLevel;
    for (auto lv : featureLevels) {
        if (D3D12CreateDevice(
            nullptr,
            lv, IID_PPV_ARGS(&_pDevice)) == S_OK
        ) {
            featureLevel = lv;
            break;
        }
    }
#pragma endregion

    // show
    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            // break loop when app quit
            break;
        }
    }

    UnregisterClass(w.lpszClassName, w.hInstance);

    debugOutputFormatString("Show window test.");
    int _ = getchar();
    return 0;
}
