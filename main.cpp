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

ID3D12Device* _device = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _dxgiSwapChain = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;

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
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
    
    // enumerate adapters by dxgi factory
    std::vector<IDXGIAdapter*> adapters;
    IDXGIAdapter* adapterCache = nullptr;
    for (int i = 0;
         _dxgiFactory->EnumAdapters(i, &adapterCache) != DXGI_ERROR_NOT_FOUND;
         i++
    ) {
        adapters.push_back(adapterCache);
    }

    // find desired adapter
    // TODO 強制的にNVIDIAを指定している
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
        result = D3D12CreateDevice(
            adapterCache,
            lv, IID_PPV_ARGS(&_device));
        if (result == S_OK) {
            featureLevel = lv;
            break;
        }
    }

    // create command allocator
    result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
    // create command list
    result = _device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _cmdAllocator,
        nullptr,
        IID_PPV_ARGS(&_cmdList)
    );

    // create command queue
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    result = _device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

    // create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = window_width;
    swapChainDesc.Height = window_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain1* dxgiSwapChain1;
    result = _dxgiFactory->CreateSwapChainForHwnd(
        _cmdQueue,
        hwnd,
        &swapChainDesc,
        nullptr, nullptr,
        &dxgiSwapChain1
    );
    // cast to DXGISwapChain4
    result = dxgiSwapChain1->QueryInterface(IID_PPV_ARGS(&_dxgiSwapChain));
    if (result == S_OK)
        dxgiSwapChain1->Release();


    
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
