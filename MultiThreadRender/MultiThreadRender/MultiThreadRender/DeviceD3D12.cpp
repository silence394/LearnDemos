#include "DeviceD3D12.h"

DeviceD3D12::DeviceD3D12(HWND hwnd) : mHwnd( hwnd )
{
}

DeviceD3D12::~DeviceD3D12()
{

}

bool DeviceD3D12::InitD3D(int width, int height)
{
	HRESULT hr;

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		return false;

	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
		return false;

	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	if (FAILED(hr))
		return false;

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = mDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&mCommandQueue));
	if (FAILED(hr))
		return false;

	// TODO. not window width height?
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = width;
	backBufferDesc.Height = height;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = mFrameBufferCount;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = mHwnd;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = true;

	IDXGISwapChain* swapChain;
	dxgiFactory->CreateSwapChain(mCommandQueue, &swapChainDesc, &swapChain);
	mSwapChain = static_cast<IDXGISwapChain3*> (swapChain);

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	RTVHeapDesc.NumDescriptors = mFrameBufferCount;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = mDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&mRTVDescriptorHeap));
	if (FAILED(hr))
		return false;

	mRTVDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < mFrameBufferCount; i++)
	{
		hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i]));
		if (FAILED(hr))
			return false;

		mDevice->CreateRenderTargetView(mRenderTargets[i], nullptr, rtvHandle);

		rtvHandle.Offset(1, mRTVDescSize);
	}

	for (int i = 0; i < mFrameBufferCount; i++)
	{
		hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[i]));
		if (FAILED(hr))
			return false;
	}

	hr = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[0], nullptr, IID_PPV_ARGS(&mCommandList));
	if (FAILED(hr))
		return false;

	mCommandList->Close();

	// Create fences.
	for (int i = 0; i < mFrameBufferCount; i++)
	{
		hr = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFences[i]));
		if (FAILED(hr))
			return false;

		mFenceValues[i] = 0;
	}

	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
		return false;

	return true;
}

void DeviceD3D12::Update()
{

}

void DeviceD3D12::UpdatePipeline()
{
	WaitForPreviousFrame();

	HRESULT hr;

	hr = mCommandAllocators[mFrameIndex]->Reset();
	if (FAILED(hr))
		;

	hr = mCommandList->Reset(mCommandAllocators[mFrameIndex], nullptr);
	if (FAILED(hr))
		;

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRTVDescSize);

	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	const float clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = mCommandList->Close();
	if (FAILED(hr))
		;
}

void DeviceD3D12::Render()
{
	UpdatePipeline();

	ID3D12CommandList* ppCommandLists[] = { mCommandList };
	// _countof(ppCommandLists)
	mCommandQueue->ExecuteCommandLists(1, ppCommandLists);

	HRESULT hr;
	hr = mCommandQueue->Signal(mFences[mFrameIndex], mFenceValues[mFrameIndex]);
	if (FAILED(hr))
		;

	hr = mSwapChain->Present(0, 0);
	if (FAILED(hr))
		;
}

void DeviceD3D12::Cleanup()
{

}

void DeviceD3D12::WaitForPreviousFrame()
{
	HRESULT hr;

	if (mFences[mFrameIndex]->GetCompletedValue() < mFenceValues[mFrameIndex])
	{
		hr = mFences[mFrameIndex]->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent);
		if (FAILED(hr))
			;

		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFenceValues[mFrameIndex] ++;

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}