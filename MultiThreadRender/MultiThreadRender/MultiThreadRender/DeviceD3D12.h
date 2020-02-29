#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

class DeviceD3D12
{
public:
	DeviceD3D12(HWND hwnd);
	~DeviceD3D12();

private:
	static const int mFrameBufferCount = 3;
	HWND mHwnd;
	ID3D12Device* mDevice;
	IDXGISwapChain3* mSwapChain;
	ID3D12CommandQueue*	mCommandQueue;
	ID3D12DescriptorHeap* mRTVDescriptorHeap;
	ID3D12Resource* mRenderTargets[mFrameBufferCount];
	ID3D12CommandAllocator* mCommandAllocators[mFrameBufferCount];
	ID3D12GraphicsCommandList* mCommandList;
	ID3D12Fence* mFences[mFrameBufferCount];

	HANDLE mFenceEvent;
	UINT64 mFenceValues[mFrameBufferCount];

	int	mFrameIndex;
	int mRTVDescSize;

public:
	bool InitD3D(int width, int height);

	void Update();

	void UpdatePipeline();

	void Render();

	void Cleanup();

	void WaitForPreviousFrame();
};