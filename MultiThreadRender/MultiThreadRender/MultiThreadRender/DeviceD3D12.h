#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
#endif
#include <vector>
#include <string>
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"


using namespace DirectX;

class DeviceD3D12
{
public:
	DeviceD3D12(HWND hwnd);
	~DeviceD3D12();

private:
	static const int mFrameBufferCount = 3;
	static const int mThreadCount = 3;
	HWND mHwnd;
	ID3D12Device* mDevice;
	IDXGISwapChain3* mSwapChain;
	ID3D12CommandQueue*	mCommandQueue;
	ID3D12DescriptorHeap* mRTVDescriptorHeap;
	ID3D12Resource* mRenderTargets[mFrameBufferCount];

	ID3D12DescriptorHeap* mGeometryRTVDescriptorHeap;
	ID3D12Resource* mGeometryRenderTarget[mThreadCount];
	ID3D12DescriptorHeap* mGeometrySRVDescriptorHeap;

	ID3D12DescriptorHeap* mGeometryDSDescriptorHeap;
	ID3D12Resource* mGeometryDepthStencil[mThreadCount];

	ID3D12CommandAllocator* mCommandAllocators[mFrameBufferCount];
	ID3D12GraphicsCommandList* mCommandList;
	ID3D12Fence* mFences[mFrameBufferCount];

	HANDLE mFenceEvent;
	UINT64 mFenceValues[mFrameBufferCount];

	// Geometry commandlists.
	ID3D12CommandAllocator* mGeoCommandAllocators[mThreadCount];
	ID3D12GraphicsCommandList* mGeoCommandLists[mThreadCount];

	int	mFrameIndex;
	int mRTVDescSize;
	int mDSVDescSize;
	int mCbvSrvDescSize;

	ID3D12PipelineState* mPipelineState;
	ID3D12RootSignature* mRootSignature;

	ID3D12RootSignature* mGeometryRootSignature;

	ID3D12PipelineState* mGeometryPipelineState;

	D3D12_VIEWPORT	mViewport;
	D3D12_RECT mScissorRect;

	struct Geometry
	{
		ID3D12Resource* mVertexBuffer;
		int mVertexBufferSize;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
		ID3D12Resource* mIndexBuffer;
		int mIndexBufferSize;
		D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
		int mIndexCount;
	};

	ID3D12Resource* mDepthStencilBuffer;
	ID3D12DescriptorHeap* mDSDescHeap;

	struct ConstantBuffer
	{
		XMFLOAT4X4 wvp;
	};

	ConstantBuffer mConstantBuffer;
	ID3D12Resource* mConstantBufferUploadHeap[mFrameBufferCount];
	UINT8* mConstantBufferGPUAddress[mFrameBufferCount];

	std::vector<ID3D12Resource*> mGeometryUploadBuffers;

	const int ConstantBufferAlignSize = (sizeof(ConstantBuffer)+255)&~255;

	XMFLOAT4X4 mViewMat;
	XMFLOAT4X4 mPerspectiveMat;

	Geometry mCubeGeo;
	Geometry mPyrimdGeo;
	Geometry mTriangleGeo;

	struct Transform
	{
		XMFLOAT4X4 mTranform;
		XMFLOAT4 mPosition;
		XMFLOAT4X4 mRotation;
	};

	Transform mCubeMat;
	Transform mPyrimdMat;
	Transform mTriangleMat;

private:
	void Draw(const Geometry& geo, ID3D12GraphicsCommandList* pCmdLis);
	ID3D12Resource* RequestGeometryUploadBuffer(int size);
	void FreeGeometryUploadBuffer(ID3D12Resource* resource);

	Geometry CreateGeometry(const void* vbuffer, int vlen, int vsize, const void* ibuffer, int ilen, int isize);

public:
	bool InitD3D(int width, int height);

	void Update();

	void UpdatePipeline();

	void Render();

	void Cleanup();

	void WaitForPreviousFrame();
};