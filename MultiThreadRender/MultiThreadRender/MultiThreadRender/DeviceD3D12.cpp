#include "DeviceD3D12.h"

DeviceD3D12::DeviceD3D12(HWND hwnd) : mHwnd( hwnd )
{
}

DeviceD3D12::~DeviceD3D12()
{
}

void DeviceD3D12::Draw(const Geometry& geo, ID3D12GraphicsCommandList* pCmdList)
{
	if (pCmdList == nullptr)
		return;

	pCmdList->IASetVertexBuffers(0, 1, &geo.mVertexBufferView);
	pCmdList->IASetIndexBuffer(&geo.mIndexBufferView);

	pCmdList->DrawIndexedInstanced(geo.mIndexCount, 1, 0, 0, 0);
}

ID3D12Resource* DeviceD3D12::RequestGeometryUploadBuffer(int size)
{
	ID3D12Resource* uploadbuffer;
	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadbuffer));
	mGeometryUploadBuffers.push_back(uploadbuffer);

	return uploadbuffer;
}

void DeviceD3D12::FreeGeometryUploadBuffer(ID3D12Resource* resource)
{
	if (resource == nullptr)
		return;

	mGeometryUploadBuffers.erase(std::find(mGeometryUploadBuffers.begin(), mGeometryUploadBuffers.end(), resource));
}

DeviceD3D12::Geometry DeviceD3D12::CreateGeometry(const void* vbuffer, int vlen, int vsize, const void* ibuffer, int ilen, int isize)
{
	Geometry geo;

	geo.mVertexBufferSize = vlen;
	geo.mIndexBufferSize = ilen;
	geo.mIndexCount = ilen / isize;

	ID3D12Resource* uploadbuffer = RequestGeometryUploadBuffer(vlen + ilen);

	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vlen), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&geo.mVertexBuffer));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = vbuffer;
	vertexData.RowPitch = vlen;
	vertexData.SlicePitch = vlen;

	UpdateSubresources(mCommandList, geo.mVertexBuffer, uploadbuffer, 0, 0, 1, &vertexData);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo.mVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(ilen), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&geo.mIndexBuffer));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = ibuffer;
	indexData.RowPitch = ilen;
	indexData.SlicePitch = ilen;

	UpdateSubresources(mCommandList, geo.mIndexBuffer, uploadbuffer, vlen, 0, 1, &indexData);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(geo.mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	geo.mVertexBufferView.BufferLocation = geo.mVertexBuffer->GetGPUVirtualAddress();
	geo.mVertexBufferView.StrideInBytes = vsize;
	geo.mVertexBufferView.SizeInBytes = geo.mVertexBufferSize;

	geo.mIndexBufferView.BufferLocation = geo.mIndexBuffer->GetGPUVirtualAddress();
	geo.mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	geo.mIndexBufferView.SizeInBytes = geo.mIndexBufferSize;

	return geo;
}

bool DeviceD3D12::InitD3D(int width, int height)
{
	HRESULT hr;

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
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

	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}

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
	mDSVDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvDescSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < mFrameBufferCount; i++)
	{
		hr = mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i]));
		if (FAILED(hr))
			return false;

		mDevice->CreateRenderTargetView(mRenderTargets[i], nullptr, rtvHandle);

		rtvHandle.Offset(1, mRTVDescSize);
	}

	// CreateGeometryHeap.
	D3D12_DESCRIPTOR_HEAP_DESC geoRTVHeapDesc = {};
	geoRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	geoRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	geoRTVHeapDesc.NumDescriptors = mThreadCount;

	hr = mDevice->CreateDescriptorHeap(&geoRTVHeapDesc, IID_PPV_ARGS(&mGeometryRTVDescriptorHeap));
	if (FAILED(hr))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC dsHeapDesc = {};
	dsHeapDesc.NumDescriptors = mThreadCount;
	dsHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	hr = mDevice->CreateDescriptorHeap(&dsHeapDesc, IID_PPV_ARGS(&mGeometryDSDescriptorHeap));
	if (FAILED(hr))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC geoSRVHeapDesc = {};
	geoSRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	geoSRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	geoSRVHeapDesc.NumDescriptors = mThreadCount;

	hr = mDevice->CreateDescriptorHeap(&geoSRVHeapDesc, IID_PPV_ARGS(&mGeometrySRVDescriptorHeap));
	if (FAILED(hr))
		return false;

	CD3DX12_RESOURCE_DESC geoRTDesc(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		width,
		height,
		1,
		1,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	CD3DX12_RESOURCE_DESC geoDSDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		width,
		height,
		1,
		1,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

	CD3DX12_CPU_DESCRIPTOR_HANDLE geoRTVHandle(mGeometryRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE geoSRVHandle(mGeometrySRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE geoDSVHandle(mGeometryDSDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = geoRTDesc.Format;
	srvDesc.Texture2D.MipLevels = geoRTDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;

	D3D12_CLEAR_VALUE clearvalue;
	clearvalue.Color[0] = 0.0f;
	clearvalue.Color[1] = 0.0f;
	clearvalue.Color[2] = 0.0f;
	clearvalue.Color[3] = 1.0f;
	clearvalue.Format = geoRTDesc.Format;

	D3D12_CLEAR_VALUE dsClearvalue;
	dsClearvalue.DepthStencil.Depth = 1.0f;
	dsClearvalue.DepthStencil.Stencil = 0;
	dsClearvalue.Format = geoDSDesc.Format;

	for (int i = 0; i < mThreadCount; i++)
	{
		hr = mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &geoRTDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearvalue, IID_PPV_ARGS(&mGeometryRenderTarget[i]));;
		if (FAILED(hr))
		{
			hr = mDevice->GetDeviceRemovedReason();
			return false;

		}
		// TODO. Desc.
		mDevice->CreateRenderTargetView(mGeometryRenderTarget[i], nullptr, geoRTVHandle);

		geoRTVHandle.Offset(1, mRTVDescSize);

		mDevice->CreateShaderResourceView(mGeometryRenderTarget[i], &srvDesc, geoSRVHandle);
		geoSRVHandle.Offset(mCbvSrvDescSize);

		hr = mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&geoDSDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&dsClearvalue,
			IID_PPV_ARGS(&mGeometryDepthStencil[i]));

		mDevice->CreateDepthStencilView(mGeometryDepthStencil[i], nullptr, geoDSVHandle);
		geoDSVHandle.Offset(mDSVDescSize);
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDSDescHeap));
	if (FAILED(hr))
		return false;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
	dsViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&mDepthStencilBuffer)
	);

	mDevice->CreateDepthStencilView(mDepthStencilBuffer, &dsViewDesc, mDSDescHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < mFrameBufferCount; i++)
	{
		hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[i]));
		if (FAILED(hr))
			return false;
	}

	hr = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[0], NULL, IID_PPV_ARGS(&mCommandList));
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

	for (int i = 0; i < mThreadCount; i++)
	{
		hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mGeoCommandAllocators[i]));
		if (FAILED(hr))
			return false;

		hr = mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mGeoCommandAllocators[i], NULL, IID_PPV_ARGS(&mGeoCommandLists[i]));

		if (FAILED(hr))
			return false;

		mGeoCommandLists[i]->Close();
	}

	D3D12_ROOT_DESCRIPTOR rootCBVDesc;
	rootCBVDesc.RegisterSpace = 0;
	rootCBVDesc.ShaderRegister = 0;

	D3D12_ROOT_PARAMETER rootParams[1];
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[0].Descriptor = rootCBVDesc;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootSignatureDesc.Init(_countof(rootParams), rootParams, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr))
		return false;

	hr = mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mGeometryRootSignature));
	if (FAILED(hr))
		return false;

	// Create tex root sigature.
	{
		D3D12_ROOT_DESCRIPTOR rootCBVDesc;
		rootCBVDesc.RegisterSpace = 0;
		rootCBVDesc.ShaderRegister = 0;

		D3D12_ROOT_PARAMETER rootParams[2];
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParams[0].Descriptor = rootCBVDesc;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		CD3DX12_DESCRIPTOR_RANGE range[1];
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		D3D12_ROOT_DESCRIPTOR_TABLE table;
		table.NumDescriptorRanges = 1;
		table.pDescriptorRanges = &range[0];

		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].DescriptorTable = table;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

		// Static sampler.
		D3D12_STATIC_SAMPLER_DESC stSamplerDesc = {};
		stSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		stSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		stSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		stSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		stSamplerDesc.MipLODBias = 0;
		stSamplerDesc.MaxAnisotropy = 0;
		stSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		stSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		stSamplerDesc.MinLOD = 0.0f;
		stSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		stSamplerDesc.ShaderRegister = 0;
		stSamplerDesc.RegisterSpace = 0;
		stSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		rootSignatureDesc.Init(_countof(rootParams), rootParams, 1, &stSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

		ID3DBlob* signature;
		hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
		if (FAILED(hr))
			return false;

		hr = mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
		if (FAILED(hr))
			return false;
	}

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

	ID3DBlob* texvs;
	hr = D3DCompileFromFile(L"texvs.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &texvs, &errorbuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*) errorbuffer->GetBufferPointer());
		return false;
	}

	D3D12_SHADER_BYTECODE texvsByteCode = {};
	texvsByteCode.BytecodeLength = texvs->GetBufferSize();
	texvsByteCode.pShaderBytecode = texvs->GetBufferPointer();

	ID3DBlob* texps;
	hr = D3DCompileFromFile(L"texps.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &texps, &errorbuffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*) errorbuffer->GetBufferPointer());
		return false;
	}

	D3D12_SHADER_BYTECODE texpsByteCode = {};
	texpsByteCode.BytecodeLength = texps->GetBufferSize();
	texpsByteCode.pShaderBytecode = texps->GetBufferPointer();

	ZeroMemory(&mConstantBuffer, sizeof(ConstantBuffer));

	for (int i = 0; i < mFrameBufferCount; i++)
	{
		hr = mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mConstantBufferUploadHeap[i]));
		if (FAILED(hr))
			return false;

		CD3DX12_RANGE readRange(0, 0);
		mConstantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&mConstantBufferGPUAddress[i]));
	}

	// Create Input Layout.
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_INPUT_LAYOUT_DESC layoutDesc = {};
	layoutDesc.pInputElementDescs = inputLayout;
	layoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);

	// Create PSO.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = layoutDesc;
	psoDesc.pRootSignature = mRootSignature;
	psoDesc.VS = texvsByteCode;
	psoDesc.PS = texpsByteCode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));
	if (FAILED(hr))
		return false;

	psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.VS = vsByteCode;
	psoDesc.PS = psByteCode;
	psoDesc.pRootSignature = mGeometryRootSignature;

	hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mGeometryPipelineState));
	if (FAILED(hr))
		return false;

	struct Vertex
	{
		float x, y, z;
		float r, g, b, a;
		float u, v;
	};

	{
		Vertex vlist[] =
		{
			{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
			{  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },

			// right side face
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
			{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

			// left side face
			{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },

			// back face
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },

			// top face
			{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },
			{ 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f },

			// bottom face
			{  0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },
		};

		DWORD ilist[] =
		{
			0, 1, 2, // first triangle
			0, 3, 1, // second triangle

			// left face
			4, 5, 6, // first triangle
			4, 7, 5, // second triangle

			// right face
			8, 9, 10, // first triangle
			8, 11, 9, // second triangle

			// back face
			12, 13, 14, // first triangle
			12, 15, 13, // second triangle

			// top face
			16, 17, 18, // first triangle
			16, 19, 17, // second triangle

			// bottom face
			20, 21, 22, // first triangle
			20, 23, 21, // second triangle
		};

		int vlen = sizeof(vlist);
		int vsize = sizeof(Vertex);
		int ilen = sizeof(ilist);
		int isize = sizeof(DWORD);

		mCubeGeo = CreateGeometry(vlist, vlen, vsize, ilist, ilen, isize);
	}
	{
		Vertex vlist[] =
		{
			{ 0.5f,  0.0f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
			{ -0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
			{ -0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
			{  0.0f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f },
		};

		DWORD ilist[] =
		{
			0, 2, 1,
			0, 3, 2,
			0, 1, 3,
			1, 2, 3
		};

		int vlen = sizeof(vlist);
		int vsize = sizeof(Vertex);
		int ilen = sizeof(ilist);
		int isize = sizeof(DWORD);

		mPyrimdGeo = CreateGeometry(vlist, vlen, vsize, ilist, ilen, isize);
	}

	{
		Vertex vlist[] =
		{
			{ 0.0f,  0.0f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.5f,  0.0f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
			{ -0.5f,  0.0f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },
		};

		DWORD ilist[] =
		{
			0, 1, 2,
			0, 2, 1,
		};

		int vlen = sizeof(vlist);
		int vsize = sizeof(Vertex);
		int ilen = sizeof(ilist);
		int isize = sizeof(DWORD);

		mTriangleGeo = CreateGeometry(vlist, vlen, vsize, ilist, ilen, isize);
	}

	mCommandList->Close();

	ID3D12CommandList* pCmdLists[] = { mCommandList };
	mCommandQueue->ExecuteCommandLists(1, pCmdLists);

	mFenceValues[mFrameIndex] ++;
	hr = mCommandQueue->Signal(mFences[mFrameIndex], mFenceValues[mFrameIndex]);
	if (FAILED(hr))
		return false;

	//for (auto i : mGeometryUploadBuffers)
	//{
	//	if (i != nullptr)
	//		i->Release();
	//}

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

	XMMATRIX tmpMat = ::XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), (float)width / (float)height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&mPerspectiveMat, tmpMat);

	XMFLOAT4 eye(0.0f, 2.0f, -4.0f, 0.0f);
	XMFLOAT4 look(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 up(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR xeye = XMLoadFloat4(&eye);
	XMVECTOR xlook= XMLoadFloat4(&look);
	XMVECTOR xup = XMLoadFloat4(&up);

	tmpMat = ::XMMatrixLookAtLH(xeye, xlook, xup);
	XMStoreFloat4x4(&mViewMat, tmpMat);

	mCubeMat.mPosition = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR posVec = XMLoadFloat4(&mCubeMat.mPosition);
	tmpMat = ::XMMatrixTranslationFromVector(posVec);
	::XMStoreFloat4x4(&mCubeMat.mRotation, XMMatrixIdentity());
	::XMStoreFloat4x4(&mCubeMat.mTranform, tmpMat);

	mPyrimdMat.mPosition = XMFLOAT4(2.0f, 0.0f, 0.0f, 0.0f);
	posVec = ::XMLoadFloat4(&mPyrimdMat.mPosition) + XMLoadFloat4(&mCubeMat.mPosition);
	tmpMat = ::XMMatrixTranslationFromVector(posVec);
	::XMStoreFloat4x4(&mPyrimdMat.mTranform, tmpMat);
	::XMStoreFloat4x4(&mPyrimdMat.mRotation, XMMatrixIdentity());

	mTriangleMat.mPosition = XMFLOAT4(0.3f, 0.0f, 0.0f, 0.0f);
	tmpMat = ::XMMatrixTranslationFromVector(posVec); 
	::XMStoreFloat4x4(&mTriangleMat.mTranform, tmpMat);
	::XMStoreFloat4x4(&mTriangleMat.mRotation, XMMatrixIdentity());

	return true;
}

void DeviceD3D12::Update()
{
	int constantBufferOffset = 0;

	// cube1.
	XMMATRIX rotXMat = ::XMMatrixRotationX(0.0000f);
	XMMATRIX rotYMat = ::XMMatrixRotationY(0.00002f);
	XMMATRIX rotZMat = ::XMMatrixRotationZ(0.0000f);

	XMMATRIX rotMat = XMLoadFloat4x4(&mCubeMat.mRotation) * rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&mCubeMat.mRotation, rotMat);

	XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&mCubeMat.mPosition));
	XMMATRIX worldMat = rotMat * translationMat;
	XMStoreFloat4x4(&mCubeMat.mTranform, worldMat);

	XMMATRIX wvp = ::XMLoadFloat4x4(&mCubeMat.mTranform) * ::XMLoadFloat4x4(&mViewMat) * ::XMLoadFloat4x4(&mPerspectiveMat);
	XMMATRIX transposed = ::XMMatrixTranspose(wvp);
	::XMStoreFloat4x4(&mConstantBuffer.wvp, transposed);

	memcpy(mConstantBufferGPUAddress[mFrameIndex], &mConstantBuffer + ConstantBufferAlignSize * constantBufferOffset ++, sizeof(mConstantBuffer));

	rotXMat = XMMatrixRotationX(0.0000f);
	rotYMat = XMMatrixRotationY(0.00002f);
	rotZMat = XMMatrixRotationZ(0.0000f);

	rotMat =(XMLoadFloat4x4(&mPyrimdMat.mRotation) * (rotXMat * rotYMat)) * rotZMat;
	::XMStoreFloat4x4(&mPyrimdMat.mRotation, rotMat);

	translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&mPyrimdMat.mPosition));

	XMMATRIX scaleMat = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	XMMATRIX nextRootMat = translationMat * rotMat;

	worldMat = scaleMat * nextRootMat;

	XMStoreFloat4x4(&mPyrimdMat.mTranform, worldMat);
	wvp = ::XMLoadFloat4x4(&mPyrimdMat.mTranform) * ::XMLoadFloat4x4(&mViewMat) * ::XMLoadFloat4x4(&mPerspectiveMat);
	transposed = ::XMMatrixTranspose(wvp);
	::XMStoreFloat4x4(&mConstantBuffer.wvp, transposed);

	// copy our ConstantBuffer instance to the mapped constant buffer resource
	memcpy(mConstantBufferGPUAddress[mFrameIndex] + ConstantBufferAlignSize * constantBufferOffset++, &mConstantBuffer, sizeof(mConstantBuffer));

	// For Triangle.
	rotXMat = XMMatrixRotationX(0.00f);
	rotYMat = XMMatrixRotationY(0.0005f);
	rotZMat = XMMatrixRotationZ(0.000f);

	rotMat =  (XMLoadFloat4x4(&mTriangleMat.mRotation) * (rotXMat * rotYMat * rotZMat));
	::XMStoreFloat4x4(&mTriangleMat.mRotation, rotMat);

	translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&mTriangleMat.mPosition));

	scaleMat = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	worldMat = scaleMat * translationMat * rotMat * nextRootMat;
	XMStoreFloat4x4(&mTriangleMat.mTranform, worldMat);

	wvp = ::XMLoadFloat4x4(&mTriangleMat.mTranform) * ::XMLoadFloat4x4(&mViewMat) * ::XMLoadFloat4x4(&mPerspectiveMat);
	transposed = ::XMMatrixTranspose(wvp);
	::XMStoreFloat4x4(&mConstantBuffer.wvp, transposed);

	// copy our ConstantBuffer instance to the mapped constant buffer resource
	memcpy(mConstantBufferGPUAddress[mFrameIndex] + ConstantBufferAlignSize * constantBufferOffset++, &mConstantBuffer, sizeof(mConstantBuffer));

	{
		// Geometry to target transform.
		{
			worldMat = XMLoadFloat4x4(&mCubeMat.mRotation);
			scaleMat = ::XMMatrixScaling(2.0f, 2.0f, 2.0f);

			wvp = scaleMat *  worldMat *::XMLoadFloat4x4(&mViewMat) * ::XMLoadFloat4x4(&mPerspectiveMat);
			transposed = ::XMMatrixTranspose(wvp);
			::XMStoreFloat4x4(&mConstantBuffer.wvp, transposed);

			memcpy(mConstantBufferGPUAddress[mFrameIndex] + ConstantBufferAlignSize * constantBufferOffset++, &mConstantBuffer, sizeof(mConstantBuffer));


			worldMat = XMLoadFloat4x4(&mPyrimdMat.mRotation);
			wvp = scaleMat * worldMat * ::XMLoadFloat4x4(&mViewMat) * ::XMLoadFloat4x4(&mPerspectiveMat);
			transposed = ::XMMatrixTranspose(wvp);
			::XMStoreFloat4x4(&mConstantBuffer.wvp, transposed);

			memcpy(mConstantBufferGPUAddress[mFrameIndex] + ConstantBufferAlignSize * constantBufferOffset++, &mConstantBuffer, sizeof(mConstantBuffer));


			worldMat = XMLoadFloat4x4(&mTriangleMat.mRotation);
			wvp = scaleMat * worldMat * ::XMLoadFloat4x4(&mViewMat) * ::XMLoadFloat4x4(&mPerspectiveMat);
			transposed = ::XMMatrixTranspose(wvp);
			::XMStoreFloat4x4(&mConstantBuffer.wvp, transposed);

			memcpy(mConstantBufferGPUAddress[mFrameIndex] + ConstantBufferAlignSize * constantBufferOffset++, &mConstantBuffer, sizeof(mConstantBuffer));
		}
	}
}

void DeviceD3D12::UpdatePipeline()
{
	WaitForPreviousFrame();

	HRESULT hr;

	for (int i = 0; i < mThreadCount; i++)
	{
		mGeoCommandAllocators[i]->Reset();
		mGeoCommandLists[i]->Reset(mGeoCommandAllocators[i], mGeometryPipelineState);
	}

	// Must before graphics desc table.
	{
		ID3D12GraphicsCommandList* pCmdList = mGeoCommandLists[0];
		pCmdList->SetGraphicsRootSignature(mGeometryRootSignature);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mGeometryRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, mRTVDescSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mGeometryDSDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, mCbvSrvDescSize);
		pCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };


		pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		pCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		pCmdList->RSSetViewports(1, &mViewport);
		pCmdList->RSSetScissorRects(1, &mScissorRect);
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCmdList->SetGraphicsRootConstantBufferView(0, mConstantBufferUploadHeap[mFrameIndex]->GetGPUVirtualAddress() + ConstantBufferAlignSize * 3);
		Draw(mCubeGeo, pCmdList);
	}

	{
		ID3D12GraphicsCommandList* pCmdList = mGeoCommandLists[1];
		pCmdList->SetGraphicsRootSignature(mGeometryRootSignature);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mGeometryRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, mRTVDescSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mGeometryDSDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, mCbvSrvDescSize);
		pCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		pCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		pCmdList->RSSetViewports(1, &mViewport);
		pCmdList->RSSetScissorRects(1, &mScissorRect);
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCmdList->SetGraphicsRootConstantBufferView(0, mConstantBufferUploadHeap[mFrameIndex]->GetGPUVirtualAddress() + ConstantBufferAlignSize * 4);
		Draw(mTriangleGeo, pCmdList);
	}

	{
		ID3D12GraphicsCommandList* pCmdList = mGeoCommandLists[1];
		pCmdList->SetGraphicsRootSignature(mGeometryRootSignature);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mGeometryRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRTVDescSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mGeometryDSDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 2, mCbvSrvDescSize);
		pCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		pCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		pCmdList->RSSetViewports(1, &mViewport);
		pCmdList->RSSetScissorRects(1, &mScissorRect);
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		pCmdList->SetGraphicsRootConstantBufferView(0, mConstantBufferUploadHeap[mFrameIndex]->GetGPUVirtualAddress() + ConstantBufferAlignSize * 5);
		Draw(mPyrimdGeo, pCmdList);
	}

	for (int i = 0; i < mThreadCount; i++)
	{
		hr = mGeoCommandLists[i]->Close();
		if (FAILED(hr))
			;
	}

	ID3D12CommandList* pCmdLists[] = { mGeoCommandLists[0], mGeoCommandLists[1], mGeoCommandLists[2] };

	mCommandQueue->ExecuteCommandLists(3, pCmdLists);

	hr = mCommandQueue->Signal(mFences[mFrameIndex], mFenceValues[mFrameIndex]);

	// TODO.
	if (mFences[mFrameIndex]->GetCompletedValue() < mFenceValues[mFrameIndex])
	{
		hr = mFences[mFrameIndex]->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent);
		if (FAILED(hr))
			;

		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFenceValues[mFrameIndex] ++;

	hr = mCommandAllocators[mFrameIndex]->Reset();
	if (FAILED(hr))
		;

	hr = mCommandList->Reset(mCommandAllocators[mFrameIndex], mPipelineState);
	if (FAILED(hr))
		;

	// Set default rendertarget.
	{
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRTVDescSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mDSDescHeap->GetCPUDescriptorHandleForHeapStart());

		mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		const float clearColor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		mCommandList->ClearDepthStencilView(mDSDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
		mCommandList->SetGraphicsRootSignature(mRootSignature);

		ID3D12DescriptorHeap* ppHeaps[] = {mGeometrySRVDescriptorHeap};
		mCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		mCommandList->RSSetViewports(1, &mViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	{
		mCommandList->SetGraphicsRootConstantBufferView(0, mConstantBufferUploadHeap[mFrameIndex]->GetGPUVirtualAddress());
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(mGeometrySRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, mRTVDescSize);
		mCommandList->SetGraphicsRootDescriptorTable(1, srvHandle);

		Draw(mPyrimdGeo, mCommandList);
	}

	{
		mCommandList->SetGraphicsRootConstantBufferView(0, mConstantBufferUploadHeap[mFrameIndex]->GetGPUVirtualAddress() + ConstantBufferAlignSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(mGeometrySRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 1, mRTVDescSize);
		mCommandList->SetGraphicsRootDescriptorTable(1, srvHandle);
		Draw(mCubeGeo, mCommandList);
	}

	{
		mCommandList->SetGraphicsRootConstantBufferView(0, mConstantBufferUploadHeap[mFrameIndex]->GetGPUVirtualAddress() + ConstantBufferAlignSize * 2);
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(mGeometrySRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 2, mRTVDescSize);
		mCommandList->SetGraphicsRootDescriptorTable(1, srvHandle);
		Draw(mTriangleGeo, mCommandList);
	}

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