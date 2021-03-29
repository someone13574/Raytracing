#pragma once

#include "../Win.h"
#include "../Macros.h"
#include "../Input/Mouse.h"

#include "UIManager.h"
#include "MeshManager.h"
#include "BoundingVolumeHierarchy.h"
#include "Camera.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "../../d3dx12.h"
#include <wrl.h>
#include <algorithm>
#include <chrono>

#include "DirectxErrorCatcher.h"

using namespace Microsoft::WRL;

namespace Graphics
{
	struct RenderConstants
	{
		float time;
		unsigned int width;
		unsigned int height;

		float originX;
		float originY;
		float originZ;

		Matrix3x3<double>::GPUMatrix3x3 cameraToWorldMatrix;
	};

	class ENGINE_DLL_DS Graphics
	{
	public:
		Graphics(HWND window, Input::Mouse* mouse, int width, int height, MeshManagement::MeshManager* meshManager, BoundingVolumeHierarchy* boundingVolumeHierarchy);
		~Graphics();
		Graphics(const Graphics&) = delete;
		Graphics& operator = (const Graphics&) = delete;
	public:
		UIManager uiManager;
		MeshManagement::MeshManager* meshManager = nullptr;
		BoundingVolumeHierarchy* boundingVolumeHierarchy = nullptr;
	public:
		bool pipelineObjectsInitialized = false;

		void Render();
		void Resize(UINT32 width, UINT32 height);
	private:
		void LoadPipeline();
		void LoadAssets();
	private:
		void EnableDebugLayer();
		void CreatePipelineDevice();
		void CreatePipelineCommandQueue();
		void CreatePipelineSwapChain();
		void CreatePipelineCommandAllocator();
		void CreatePipelineCommandList();
		void CreatePipelineSynchronizationObjects();
		void CreatePipelineRootSignature();
		void CreatePipelineStateObjects();
	private:
		void CreateBackbuffers();
		void CreateRenderTexture();
		void UpdateUIBuffer();
		void UpdateTriangleBuffer();
	private:
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT numDescriptors);
		void WaitForPreviousFrame();
	private:
		void SetRenderConstants();
		void QueueCommands();
	private:
		int clientWidth;
		int clientHeight;
		HWND wnd;
	private:
		static const UINT frameCount = 2;
		bool VSyncEnabled = false;
		bool useWarp = false;
	private:
		bool tearingSupported = false;
#pragma warning(push)
#pragma warning(disable:4251)
		//Pipeline objects
		ComPtr<ID3D12Device> pDevice;
		ComPtr<ID3D12CommandQueue> pCommandQueue;
		ComPtr<IDXGISwapChain3> pSwapChain;
		ComPtr<ID3D12Resource> pBackBuffers[frameCount];
		ComPtr<ID3D12GraphicsCommandList> pCommandList;
		ComPtr<ID3D12CommandAllocator> pCommandAllocator;
		ComPtr<ID3D12DescriptorHeap> pBackBuffersHeap;

		ComPtr<ID3D12Resource> pUnorderedAccess;
		ComPtr<ID3D12DescriptorHeap> pUAVHeap;

		ComPtr<ID3D12Resource> uiElementBuffer;
		ComPtr<ID3D12Resource> uiUploadBuffer;
		ComPtr<ID3D12DescriptorHeap> uiElementDescriptorHeap;

		ComPtr<ID3D12Resource> triangleBuffer;
		ComPtr<ID3D12Resource> triangleUploadBuffer;
		ComPtr<ID3D12DescriptorHeap> triangleDescriptorHeap;

		ComPtr<ID3D12Resource> boundingVolumeHierarchyBuffer;
		ComPtr<ID3D12Resource> boundingVolumeHierarchyUploadBuffer;
		ComPtr<ID3D12DescriptorHeap> boundingVolumeHierarchyDescriptorHeap;

		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> vertexUploadBuffer;
		ComPtr<ID3D12DescriptorHeap> vertexDescriptorHeap;

		UINT RTVDescriptorSize;
		UINT UAVDescriptorSize;
		UINT currentBackBufferIndex;

		ComPtr<ID3D12PipelineState> pRenderPipelineState;

		ComPtr<ID3D12RootSignature> pRootSignature;

		//Synchronization objects
		ComPtr<ID3D12Fence> pFence;
		UINT64 fenceValue = 1;
		UINT64 frameFenceValues[frameCount] = {};
		HANDLE fenceEvent;

		Camera camera = Camera();

#ifndef NDEBUG
		EngineDebug::DirectxErrorCatcher errorCatcher;
#endif
#pragma warning(pop)
	};
}

