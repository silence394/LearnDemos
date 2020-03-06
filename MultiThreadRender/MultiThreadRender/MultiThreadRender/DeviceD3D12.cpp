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

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr))
		return false;

	hr = mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
	if (FAILED(hr))
		return false;

	ID3DBlob* vs;
	ID3DBlob* errorbuffer;
	hr = D3DCompileFromFile(L"vs.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vs, &errorbuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorbuffer->GetBufferPointer());
		return false;
	}

	D3D12_SHADER_BYTECODE vsByteCode = {};
	vsByteCode.BytecodeLength = vs->GetBufferSize();
	vsByteCode.pShaderBytecode = vs->GetBufferPointer();

	ID3DBlob* ps;
	hr = D3DCompileFromFile(L"ps.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &ps, &errorbuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorbuffer->GetBufferPointer());
		return false;
	}

	D3D12_SHADER_BYTECODE psByteCode = {};
	psByteCode.BytecodeLength = ps->GetBufferSize();
	psByteCode.pShaderBytecode = ps->GetBufferPointer();

	// Create Input Layout.
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_INPUT_LAYOUT_DESC layoutDesc = {};
	layoutDesc.pInputElementDescs = inputLayout;
	layoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);

	// Create PSO.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = layoutDesc;
	psoDesc.pRootSignature = mRootSignature;
	psoDesc.VS = vsByteCode;
	psoDesc.PS = psByteCode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;

	hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));
	if (FAILED(hr))
		return false;

	struct Vertex
	{
		float x, y, z;
		float r, g, b, a;
	};

	Vertex vlist[] =
	{
		{0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f},
		{0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
		{-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f},
	};

	int vSize = sizeof(vlist);
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vSize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mVertexBuffer));

	ID3D12Resource* vBuffeUpload;
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vBuffeUpload));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vlist);
	vertexData.RowPitch = vSize;
	vertexData.SlicePitch = vSize;

	UpdateSubresources(mCommandList, mVertexBuffer, vBuffeUpload, 0, 0, 1, &vertexData);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	DWORD ilist[] =
	{
		0, 1, 2,
		0, 3, 1,
	};

	int isize = sizeof(ilist);
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(isize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mIndexBuffer));

	ID3D12Resource* iBufferUpload;
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(isize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&iBufferUpload));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(ilist);
	indexData.RowPitch = isize;
	indexData.SlicePitch = isize;

	UpdateSubresources(mCommandList, mIndexBuffer, iBufferUpload, 0, 0, 1, &indexData);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	mCommandList->Close();

	ID3D12CommandList* pCmdLists[] = { mCommandList };
	mCommandQueue->ExecuteCommandLists(1, pCmdLists);

	mFenceValues[mFrameIndex] ++;
	hr = mCommandQueue->Signal(mFences[mFrameIndex], mFenceValues[mFrameIndex]);
	if (FAILED(hr))
		return false;

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = vSize;

	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = isize;

	mViewport.TopLeftX = 0;
	mViewport.TopLeftY = 0;
	mViewport.Width = width;
	mViewport.Height = height;
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	mScissorRect.left = 0;
	mScissorRect.top = 0;
	mScissorRect.right = width;
	mScissorRect.bottom = height;

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

	hr = mCommandList->Reset(mCommandAllocators[mFrameIndex], mPipelineState);
	if (FAILED(hr))
		;

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRTVDescSize);

	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	const float clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	mCommandList->SetGraphicsRootSignature(mRootSignature);
	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	mCommandList->IASetIndexBuffer(&mIndexBufferView);
	mCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

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

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	if (mFences[mFrameIndex]->GetCompletedValue() < mFenceValues[mFrameIndex])
	{
		hr = mFences[mFrameIndex]->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent);
		if (FAILED(hr))
			;

		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFenceValues[mFrameIndex] ++;
}