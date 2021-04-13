#include "Graphics.h"
#include "EngineLogger.h"
#include "Exceptions/EngineGFXHresultException.h"
#include "Exceptions/EngineWndHresultException.h"
#include "Exceptions/EngineGFXInfoException.h"
#include "Exceptions/EngineGFXDeviceRemovedException.h"
#include "MeshManager.h"

#include <math.h>
#include <string>
#include <sstream>

#pragma comment(lib, "D3DCompiler.lib")

namespace Graphics
{
	Graphics::Graphics(HWND window, Input::Mouse* mouse, int width, int height, MeshManagement::MeshManager* meshManager) : clientWidth(width), clientHeight(height), wnd(window), uiManager(UIManager(mouse)), meshManager(meshManager), mouse(mouse)
	{
		LoadPipeline();
		LoadAssets();

		pipelineObjectsInitialized = true;
	}

	Graphics::~Graphics()
	{
		if (pipelineObjectsInitialized)
		{
			WaitForPreviousFrame();
			CloseHandle(fenceEvent);
		}
	}

#pragma region Pipeline Creation Methods
	void Graphics::EnableDebugLayer()
	{
#ifndef NDEBUG
		HRESULT hr;

		//Create Debug Interface
		ComPtr<ID3D12Debug> debugInterface;
		GFX_THROW_INFO(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));

		//Enable Debug Layer
		debugInterface->EnableDebugLayer();
#endif
	}

	void Graphics::CreatePipelineDevice()
	{
		HRESULT hr;

		//Create Factory
		ComPtr<IDXGIFactory4> dxgiFactory;
		GFX_THROW_INFO(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

		//Create using Windows Advanced Rasterization Platform or Hardware
		if (useWarp)
		{
			//Get Adapter using WARP
			ComPtr<IDXGIAdapter> warpAdapter;
			GFX_THROW_INFO(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			//Create Device
			GFX_THROW_INFO(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice)));
		}
		else
		{
			ComPtr<IDXGIAdapter1> intermediateAdapter;
			ComPtr<IDXGIAdapter4> hardwareAdapter;
			SIZE_T maxDedicatedVideoMemory = 0;
			
			//Loop through all adapters
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &intermediateAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				//Get Adapter Description
				DXGI_ADAPTER_DESC1 adapterDescription;
				intermediateAdapter->GetDesc1(&adapterDescription);

				//Test If Adapter is Hardware Adapter
				if ((adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
				{
					//Test If Creating Device With Adapter With Feature Level 12.1 Succeeds
					if (SUCCEEDED(D3D12CreateDevice(intermediateAdapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)))
					{
						//Test If Adapter Has More Memory Then Previous Best
						if (adapterDescription.DedicatedVideoMemory > maxDedicatedVideoMemory)
						{
							//Set Max Video Memory
							maxDedicatedVideoMemory = adapterDescription.DedicatedVideoMemory;

							//Cast Intermediate Adapter To Final Adapter
							GFX_THROW_INFO(intermediateAdapter.As(&hardwareAdapter));
						}
					}
				}
			}
			//Test If Valid Hardware Adapter Was Found
			if (maxDedicatedVideoMemory == 0)
			{
				//Enabled and Create WARP Device
				useWarp = true;
				CreatePipelineDevice();

				//Exit
				return;
			}

			//Create Device Using Best Adapter
			GFX_THROW_INFO(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice)));
		}
	}

	void Graphics::CreatePipelineCommandQueue()
	{
		HRESULT hr;

		//Create Command Queue Description Structure
		D3D12_COMMAND_QUEUE_DESC commandQueueDescription = {};
		commandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDescription.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		//Create Command Queue
		GFX_THROW_INFO(pDevice->CreateCommandQueue(&commandQueueDescription, IID_PPV_ARGS(&pCommandQueue)));
	}

	void Graphics::CreatePipelineSwapChain()
	{
		HRESULT hr;

		//Create Factory
		ComPtr<IDXGIFactory4> dxgiFactory;
		GFX_THROW_INFO(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

		//Create Swap Chain Description Structure
		DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
		swapChainDescription.BufferDesc.Width = clientWidth;
		swapChainDescription.BufferDesc.Height = clientHeight;
		swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.BufferCount = frameCount;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDescription.OutputWindow = wnd;
		swapChainDescription.Windowed = true;

		//Create Intermediate Swap Chain
		ComPtr<IDXGISwapChain> intermediateSwapChain;
		GFX_THROW_INFO(dxgiFactory->CreateSwapChain(pCommandQueue.Get(), &swapChainDescription, &intermediateSwapChain));

		//Cast Intermediate Swap Chain To Swap Chain
		GFX_THROW_INFO(intermediateSwapChain.As(&pSwapChain));

		//Disable Fullscreen Switching
		GFX_THROW_INFO(dxgiFactory->MakeWindowAssociation(wnd, DXGI_MWA_NO_ALT_ENTER));

		//Set The Current Back Buffer Index
		currentBackBufferIndex = pSwapChain->GetCurrentBackBufferIndex();
	}

	void Graphics::CreatePipelineCommandAllocator()
	{
		HRESULT hr;

		//Create Command Allocator
		GFX_THROW_INFO(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator)));
	}

	void Graphics::CreatePipelineCommandList()
	{
		HRESULT hr;

		//Create Command List
		GFX_THROW_INFO(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList)));

		//Close Command List Commands
		GFX_THROW_INFO(pCommandList->Close());
	}

	void Graphics::CreatePipelineSynchronizationObjects()
	{
		HRESULT hr;

		//Create Fence
		GFX_THROW_INFO(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));

		//Create Event Handle
		fenceEvent = CreateEvent(nullptr, false, false, nullptr);
		if (fenceEvent == nullptr)
			WND_LAST_EXCEPT();
	}

	void Graphics::CreatePipelineRootSignature()
	{
		HRESULT hr;

		//Root Parameter for Unordered Access View which is rendered to
		D3D12_DESCRIPTOR_RANGE renderTextureDescriptorRange;
		ZeroMemory(&renderTextureDescriptorRange, sizeof(renderTextureDescriptorRange));
		renderTextureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		renderTextureDescriptorRange.NumDescriptors = 1;
		renderTextureDescriptorRange.BaseShaderRegister = 0;
		renderTextureDescriptorRange.RegisterSpace = 0;
		renderTextureDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER renderTextureRootParameter;
		ZeroMemory(&renderTextureRootParameter, sizeof(renderTextureRootParameter));
		renderTextureRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		renderTextureRootParameter.DescriptorTable = { 1, &renderTextureDescriptorRange };
		renderTextureRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter temporary history buffer
		D3D12_DESCRIPTOR_RANGE tempTextureDescriptorRange;
		ZeroMemory(&tempTextureDescriptorRange, sizeof(tempTextureDescriptorRange));
		tempTextureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		tempTextureDescriptorRange.NumDescriptors = 1;
		tempTextureDescriptorRange.BaseShaderRegister = 3;
		tempTextureDescriptorRange.RegisterSpace = 0;
		tempTextureDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER tempTextureRootParameter;
		ZeroMemory(&tempTextureRootParameter, sizeof(tempTextureRootParameter));
		tempTextureRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		tempTextureRootParameter.DescriptorTable = { 1, &tempTextureDescriptorRange };
		tempTextureRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter history buffer
		D3D12_DESCRIPTOR_RANGE reprojectionBufferDescriptorRange;
		ZeroMemory(&reprojectionBufferDescriptorRange, sizeof(reprojectionBufferDescriptorRange));
		reprojectionBufferDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		reprojectionBufferDescriptorRange.NumDescriptors = 1;
		reprojectionBufferDescriptorRange.BaseShaderRegister = 4;
		reprojectionBufferDescriptorRange.RegisterSpace = 0;
		reprojectionBufferDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER reprojectionBufferRootParameter;
		ZeroMemory(&reprojectionBufferRootParameter, sizeof(reprojectionBufferRootParameter));
		reprojectionBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		reprojectionBufferRootParameter.DescriptorTable = { 1, &reprojectionBufferDescriptorRange };
		reprojectionBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter for geomerty history buffer
		D3D12_DESCRIPTOR_RANGE geomertyHistoryBufferDescriptorRange;
		ZeroMemory(&geomertyHistoryBufferDescriptorRange, sizeof(geomertyHistoryBufferDescriptorRange));
		geomertyHistoryBufferDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		geomertyHistoryBufferDescriptorRange.NumDescriptors = 1;
		geomertyHistoryBufferDescriptorRange.BaseShaderRegister = 5;
		geomertyHistoryBufferDescriptorRange.RegisterSpace = 0;
		geomertyHistoryBufferDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER geomertyHistoryBufferRootParameter;
		ZeroMemory(&geomertyHistoryBufferRootParameter, sizeof(geomertyHistoryBufferRootParameter));
		geomertyHistoryBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		geomertyHistoryBufferRootParameter.DescriptorTable = { 1, &geomertyHistoryBufferDescriptorRange };
		geomertyHistoryBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter for geomerty history buffer
		D3D12_DESCRIPTOR_RANGE temporaryGeomertyHistoryBufferDescriptorRange;
		ZeroMemory(&temporaryGeomertyHistoryBufferDescriptorRange, sizeof(temporaryGeomertyHistoryBufferDescriptorRange));
		temporaryGeomertyHistoryBufferDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		temporaryGeomertyHistoryBufferDescriptorRange.NumDescriptors = 1;
		temporaryGeomertyHistoryBufferDescriptorRange.BaseShaderRegister = 6;
		temporaryGeomertyHistoryBufferDescriptorRange.RegisterSpace = 0;
		temporaryGeomertyHistoryBufferDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER temporaryGeomertyHistoryBufferRootParameter;
		ZeroMemory(&temporaryGeomertyHistoryBufferRootParameter, sizeof(temporaryGeomertyHistoryBufferRootParameter));
		temporaryGeomertyHistoryBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		temporaryGeomertyHistoryBufferRootParameter.DescriptorTable = { 1, &temporaryGeomertyHistoryBufferDescriptorRange };
		temporaryGeomertyHistoryBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter for UI Element Buffer
		D3D12_DESCRIPTOR_RANGE uiDescriptorRange;
		ZeroMemory(&uiDescriptorRange, sizeof(uiDescriptorRange));
		uiDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		uiDescriptorRange.NumDescriptors = 1;
		uiDescriptorRange.BaseShaderRegister = 0;
		uiDescriptorRange.RegisterSpace = 0;
		uiDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER uiBufferRootParameter;
		ZeroMemory(&uiBufferRootParameter, sizeof(uiBufferRootParameter));
		uiBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		uiBufferRootParameter.DescriptorTable = { 1, &uiDescriptorRange };
		uiBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter for constants
		D3D12_ROOT_PARAMETER constantsRootParameter;
		ZeroMemory(&constantsRootParameter, sizeof(constantsRootParameter));
		constantsRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		constantsRootParameter.Constants = { 0, 0, (UINT)std::ceil(sizeof(RenderConstants) / 4) };
		constantsRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter for Triangle Buffer
		D3D12_DESCRIPTOR_RANGE triangleBufferDescriptorRange;
		ZeroMemory(&triangleBufferDescriptorRange, sizeof(triangleBufferDescriptorRange));
		triangleBufferDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		triangleBufferDescriptorRange.NumDescriptors = 1;
		triangleBufferDescriptorRange.BaseShaderRegister = 1;
		triangleBufferDescriptorRange.RegisterSpace = 0;
		triangleBufferDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER triangleBufferRootParameter;
		ZeroMemory(&triangleBufferRootParameter, sizeof(triangleBufferRootParameter));
		triangleBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		triangleBufferRootParameter.DescriptorTable = { 1, &triangleBufferDescriptorRange };
		triangleBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root Parameter for Bounding Volume Hierarchy Node Buffer
		D3D12_DESCRIPTOR_RANGE bvhNodeBufferDescriptorRange;
		ZeroMemory(&bvhNodeBufferDescriptorRange, sizeof(bvhNodeBufferDescriptorRange));
		bvhNodeBufferDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		bvhNodeBufferDescriptorRange.NumDescriptors = 1;
		bvhNodeBufferDescriptorRange.BaseShaderRegister = 1;
		bvhNodeBufferDescriptorRange.RegisterSpace = 0;
		bvhNodeBufferDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER bvhNodeBufferRootParameter;
		ZeroMemory(&bvhNodeBufferRootParameter, sizeof(bvhNodeBufferRootParameter));
		bvhNodeBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		bvhNodeBufferRootParameter.DescriptorTable = { 1, &bvhNodeBufferDescriptorRange };
		bvhNodeBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Root parameter for vertex buffer
		D3D12_DESCRIPTOR_RANGE vertexBufferDescriptorRange;
		ZeroMemory(&vertexBufferDescriptorRange, sizeof(vertexBufferDescriptorRange));
		vertexBufferDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		vertexBufferDescriptorRange.NumDescriptors = 1;
		vertexBufferDescriptorRange.BaseShaderRegister = 2;
		vertexBufferDescriptorRange.RegisterSpace = 0;
		vertexBufferDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER vertexBufferRootParameter;
		ZeroMemory(&vertexBufferRootParameter, sizeof(vertexBufferRootParameter));
		vertexBufferRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		vertexBufferRootParameter.DescriptorTable = { 1, &vertexBufferDescriptorRange };
		vertexBufferRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		//Create Root Parameter Array
		D3D12_ROOT_PARAMETER rootParameters[10] = { renderTextureRootParameter, uiBufferRootParameter, constantsRootParameter, triangleBufferRootParameter, bvhNodeBufferRootParameter, vertexBufferRootParameter, tempTextureRootParameter, reprojectionBufferRootParameter, geomertyHistoryBufferRootParameter, temporaryGeomertyHistoryBufferRootParameter };

		//Create Root Signature Descriptor Structure
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
		ZeroMemory(&rootSignatureDescriptor, sizeof(rootSignatureDescriptor));
		rootSignatureDescriptor.NumParameters = _countof(rootParameters);
		rootSignatureDescriptor.pParameters = rootParameters;
		rootSignatureDescriptor.NumStaticSamplers = 0;
		rootSignatureDescriptor.pStaticSamplers = nullptr;
		rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		//Serialize and Create Root Signature
		ComPtr<ID3DBlob> signatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		GFX_THROW_INFO(D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob));
		GFX_THROW_INFO(pDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)));
	}

	void Graphics::CreatePipelineStateObjects()
	{
		HRESULT hr;

		//Compile
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_DEBUG; //D3DCOMPILE_WARNINGS_ARE_ERRORS

		char buffer[8];
		_itoa_s(meshManager->GetMesh(0).rootIndex, buffer, 8, 10);
		std::string rootNodeIndexStr = std::string(buffer);

		D3D_SHADER_MACRO rootNodeIndexMacro = { "ROOT_NODE_INDEX", rootNodeIndexStr.c_str() };

		D3D_SHADER_MACRO defines[] = { rootNodeIndexMacro,  { NULL, NULL } };

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT shaderHR = D3DCompileFromFile(L"C:/Users/Owen/Documents/C++/RaytracingEngine/Engine/src/Engine/Graphics/Shaders/RenderCompute.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_1", flags, 0, &shaderBlob, &errorBlob);

		if (FAILED(shaderHR) && errorBlob)
		{
			const char* errorMsg = (const char*)errorBlob->GetBufferPointer();
			MessageBox(nullptr, errorMsg, "Shader Compilation Error", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
			PostQuitMessage(-1);
			ExitProcess(-1);
		}

		//Create Description Structure For Render Pipeline State
		D3D12_COMPUTE_PIPELINE_STATE_DESC renderPipelineStateDescription;
		ZeroMemory(&renderPipelineStateDescription, sizeof(renderPipelineStateDescription));
		renderPipelineStateDescription.pRootSignature = pRootSignature.Get();
		renderPipelineStateDescription.CS = { reinterpret_cast<UINT8*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize() };
		renderPipelineStateDescription.NodeMask = 0;
		renderPipelineStateDescription.CachedPSO = { NULL, 0 };
		renderPipelineStateDescription.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		//Create Pipeline State
		GFX_THROW_INFO(pDevice->CreateComputePipelineState(&renderPipelineStateDescription, IID_PPV_ARGS(&pRenderPipelineState)));
	}
#pragma endregion

	void Graphics::LoadPipeline()
	{
		EnableDebugLayer();
		CreatePipelineDevice();
		CreatePipelineCommandQueue();
		CreatePipelineSwapChain();
		CreatePipelineCommandAllocator();
		CreatePipelineCommandList();
		CreatePipelineSynchronizationObjects();
		CreatePipelineRootSignature();
		CreatePipelineStateObjects();
	}

	void Graphics::LoadAssets()
	{
		CreateBackbuffers();
		CreateRenderTextures();

		WaitForPreviousFrame();
	}

	ComPtr<ID3D12DescriptorHeap> Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT numDescriptors)
	{
		HRESULT hr;

		//Create Descriptor Heap Description Structure
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescription;
		ZeroMemory(&descriptorHeapDescription, sizeof(descriptorHeapDescription));
		descriptorHeapDescription.Type = type;
		descriptorHeapDescription.NumDescriptors = numDescriptors;
		descriptorHeapDescription.Flags = flags;

		//Create Descriptor Heap
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		GFX_THROW_INFO(pDevice->CreateDescriptorHeap(&descriptorHeapDescription, IID_PPV_ARGS(&descriptorHeap)));

		return descriptorHeap;
	}

	void Graphics::CreateBackbuffers()
	{
		HRESULT hr;

		pBackBuffersHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, frameCount);
		RTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//Create Backbuffer Textures
		auto rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(pBackBuffersHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < frameCount; i++)
		{
			GFX_THROW_INFO(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffers[i])));
			pDevice->CreateRenderTargetView(pBackBuffers[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, rtvDescriptorSize);
		}
	}

	void Graphics::CreateRenderTextures()
	{
		{
			HRESULT hr;

			//Create Resource Description Structure
			D3D12_RESOURCE_DESC resourceDescription = {};
			resourceDescription.DepthOrArraySize = 1;
			resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			resourceDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceDescription.Width = clientWidth;
			resourceDescription.Height = clientHeight;
			resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDescription.MipLevels = 1;
			resourceDescription.SampleDesc.Count = 1;

			//Create Heap Properties Structure
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0;
			heapProperties.VisibleNodeMask = 0;

			//Create Committed Resource
			GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pUnorderedAccess)));

			//Create Descriptor Heap
			pUAVHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

			//Create Unordered Access View Description Structure
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

			//Create Render Texture
			pDevice->CreateUnorderedAccessView(pUnorderedAccess.Get(), nullptr, &uavDesc, pUAVHeap->GetCPUDescriptorHandleForHeapStart());
		}

		{
			HRESULT hr;

			//Create Resource Description Structure
			D3D12_RESOURCE_DESC resourceDescription = {};
			resourceDescription.DepthOrArraySize = 1;
			resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			resourceDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceDescription.Width = clientWidth;
			resourceDescription.Height = clientHeight;
			resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDescription.MipLevels = 1;
			resourceDescription.SampleDesc.Count = 1;

			//Create Heap Properties Structure
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0;
			heapProperties.VisibleNodeMask = 0;

			//Create Committed Resource
			GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pTemporaryHistoryBuffer)));
			GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pGeomertyTemporaryHistoryBuffer)));

			//Create Descriptor Heap
			pTemporaryHistoryBufferHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);
			pGeomertyTemporaryHistoryBufferHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

			//Create Unordered Access View Description Structure
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

			//Create Render Texture
			pDevice->CreateUnorderedAccessView(pTemporaryHistoryBuffer.Get(), nullptr, &uavDesc, pTemporaryHistoryBufferHeap->GetCPUDescriptorHandleForHeapStart());
			pDevice->CreateUnorderedAccessView(pGeomertyTemporaryHistoryBuffer.Get(), nullptr, &uavDesc, pGeomertyTemporaryHistoryBufferHeap->GetCPUDescriptorHandleForHeapStart());
		}

		{
			HRESULT hr;

			//Create Resource Description Structure
			D3D12_RESOURCE_DESC resourceDescription = {};
			resourceDescription.DepthOrArraySize = 1;
			resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resourceDescription.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			resourceDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resourceDescription.Width = clientWidth;
			resourceDescription.Height = clientHeight;
			resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDescription.MipLevels = 1;
			resourceDescription.SampleDesc.Count = 1;

			//Create Heap Properties Structure
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0;
			heapProperties.VisibleNodeMask = 0;

			//Create Committed Resource
			GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pHistoryBuffer)));
			GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pGeomertyHistoryBuffer)));

			//Create Descriptor Heap
			pHistoryBufferHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);
			pGeomertyHistoryBufferHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

			//Create Unordered Access View Description Structure
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

			//Create Render Texture
			pDevice->CreateUnorderedAccessView(pHistoryBuffer.Get(), nullptr, &uavDesc, pHistoryBufferHeap->GetCPUDescriptorHandleForHeapStart());
			pDevice->CreateUnorderedAccessView(pGeomertyHistoryBuffer.Get(), nullptr, &uavDesc, pGeomertyHistoryBufferHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}

	void Graphics::UpdateUIBuffer()
	{
		if (uiManager.ElementCount() != 0)
		{
			HRESULT hr;

			std::vector<UIElement::GraphicsUIElement> uiElements = uiManager.GetElementData();
			if (!uiManager.IsUpdateToDate())
			{
				bool updateOnly = uiManager.UpdateOnly();

				uiManager.MarkAsUpToDate();

				//Get Buffer and element size
				UINT elementSize{ static_cast<UINT>(sizeof(UIElement::GraphicsUIElement)) };
				UINT bufferSize{ static_cast<UINT>(uiElements.size() * elementSize) };

				//Create Heap Properties Structure
				D3D12_HEAP_PROPERTIES heapProperties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };

				//Create Resource Description
				CD3DX12_RESOURCE_DESC resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

				if (!updateOnly)
				{
					//Create Committed Resource and Descriptor Heap
					GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&uiElementBuffer)));
					uiElementDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

					//Create UI Buffer
					D3D12_SHADER_RESOURCE_VIEW_DESC bufferDescriptor;
					ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
					bufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
					bufferDescriptor.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					bufferDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					bufferDescriptor.Buffer = { 0, uiManager.ElementCount(), elementSize, D3D12_BUFFER_SRV_FLAG_NONE };

					static UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = uiElementDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					pDevice->CreateShaderResourceView(uiElementBuffer.Get(), &bufferDescriptor, cpuHandle);

					heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

					GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uiUploadBuffer)));
				}

				void* pData;
				GFX_THROW_INFO(uiUploadBuffer->Map(0, NULL, &pData));
				memcpy(pData, uiElements.data(), bufferSize);
				uiUploadBuffer->Unmap(0, NULL);
				pCommandList->CopyBufferRegion(uiElementBuffer.Get(), 0, uiUploadBuffer.Get(), 0, bufferSize);
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(uiElementBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				pCommandList->ResourceBarrier(1, &barrier);
			}
		}
	}

	void Graphics::UpdateTriangleBuffer()
	{
		HRESULT hr;

		if (!meshManager->IsUpToDate())
		{
			{
				std::vector<Mesh::Triangle> triangleData = meshManager->GetTriangleArray();

				UINT elementSize{ static_cast<UINT>(sizeof(Mesh::Triangle)) };
				UINT bufferSize{ static_cast<UINT>(triangleData.size() * elementSize) };

				D3D12_HEAP_PROPERTIES heapProperties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };

				CD3DX12_RESOURCE_DESC resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

				GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&triangleBuffer)));
				triangleDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

				D3D12_SHADER_RESOURCE_VIEW_DESC bufferDescriptor;
				ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
				bufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
				bufferDescriptor.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				bufferDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				bufferDescriptor.Buffer = { 0, (UINT)triangleData.size(), elementSize, D3D12_BUFFER_SRV_FLAG_NONE };

				static UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				pDevice->CreateShaderResourceView(triangleBuffer.Get(), &bufferDescriptor, triangleDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

				heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

				GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&triangleUploadBuffer)));

				void* pData;
				GFX_THROW_INFO(triangleUploadBuffer->Map(0, NULL, &pData));
				memcpy(pData, triangleData.data(), bufferSize);
				triangleUploadBuffer->Unmap(0, NULL);
				pCommandList->CopyBufferRegion(triangleBuffer.Get(), 0, triangleUploadBuffer.Get(), 0, bufferSize);
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(triangleBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				pCommandList->ResourceBarrier(1, &barrier);
			}

			{
				//Update Vertex Data
				std::vector<Mesh::Vertex> vertexData = meshManager->GetVertexArray();

				UINT elementSize{ static_cast<UINT>(sizeof(Mesh::Vertex)) };
				UINT bufferSize{ static_cast<UINT>(vertexData.size() * elementSize) };

				D3D12_HEAP_PROPERTIES heapProperties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
				CD3DX12_RESOURCE_DESC resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
				GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer)));
				vertexDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

				D3D12_UNORDERED_ACCESS_VIEW_DESC bufferDescriptor;
				ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
				bufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
				bufferDescriptor.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				bufferDescriptor.Buffer = { 0, (UINT)vertexData.size(), elementSize, 0, D3D12_BUFFER_UAV_FLAG_NONE };

				static UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				pDevice->CreateUnorderedAccessView(vertexBuffer.Get(), nullptr, &bufferDescriptor, vertexDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

				heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
				resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

				GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexUploadBuffer)));

				void* pData;
				GFX_THROW_INFO(vertexUploadBuffer->Map(0, NULL, &pData));
				memcpy(pData, vertexData.data(), bufferSize);
				vertexUploadBuffer->Unmap(0, NULL);
				pCommandList->CopyBufferRegion(vertexBuffer.Get(), 0, vertexUploadBuffer.Get(), 0, bufferSize);
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				pCommandList->ResourceBarrier(1, &barrier);
			}

			{
				//Update BVH
				UINT bvhElementSize{ static_cast<UINT>(sizeof(Mesh::LinkedNode)) };
				UINT bvhNodeBufferSize{ static_cast<UINT>(meshManager->GetMesh(0).nodeHierarchy.size() * bvhElementSize) };

				D3D12_HEAP_PROPERTIES heapProperties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };

				CD3DX12_RESOURCE_DESC resourceDescription = CD3DX12_RESOURCE_DESC::Buffer(bvhNodeBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

				GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&boundingVolumeHierarchyBuffer)));
				boundingVolumeHierarchyDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1);

				D3D12_UNORDERED_ACCESS_VIEW_DESC bufferDescriptor;
				ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
				bufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
				bufferDescriptor.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				bufferDescriptor.Buffer = { 0, (UINT)meshManager->GetMesh(0).nodeHierarchy.size(), bvhElementSize, 0, D3D12_BUFFER_UAV_FLAG_NONE };

				static UINT descriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				pDevice->CreateUnorderedAccessView(boundingVolumeHierarchyBuffer.Get(), nullptr, &bufferDescriptor, boundingVolumeHierarchyDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

				heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
				resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

				GFX_THROW_INFO(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&boundingVolumeHierarchyUploadBuffer)));

				void* pData;
				GFX_THROW_INFO(boundingVolumeHierarchyUploadBuffer->Map(0, NULL, &pData));
				memcpy(pData, meshManager->GetMesh(0).GetLinkedNodeHierarchy().data(), bvhNodeBufferSize);
				boundingVolumeHierarchyUploadBuffer->Unmap(0, NULL);
				pCommandList->CopyBufferRegion(boundingVolumeHierarchyBuffer.Get(), 0, boundingVolumeHierarchyUploadBuffer.Get(), 0, bvhNodeBufferSize);
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(boundingVolumeHierarchyBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				pCommandList->ResourceBarrier(1, &barrier);
			}
		}
	}

	void Graphics::WaitForPreviousFrame()
	{
		HRESULT hr;

		const UINT64 fenceValueForSignal = fenceValue;
		GFX_THROW_INFO(pCommandQueue->Signal(pFence.Get(), fenceValueForSignal));
		fenceValue++;

		if (pFence->GetCompletedValue() < fenceValueForSignal)
		{
			GFX_THROW_INFO(pFence->SetEventOnCompletion(fenceValueForSignal, fenceEvent));
			WaitForSingleObject(fenceEvent, INFINITE);
		}

		currentBackBufferIndex = pSwapChain->GetCurrentBackBufferIndex();
	}

	void Graphics::Resize(UINT32 width, UINT32 height)
	{
		HRESULT hr;

		if (clientWidth != width || clientHeight != height)
		{
			clientWidth = std::max(1u, width);
			clientHeight = std::max(1u, height);

			WaitForPreviousFrame();

			pUnorderedAccess.Reset();
			for (int i = 0; i < frameCount; i++)
			{
				pBackBuffers[i].Reset();
			}

			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			GFX_THROW_INFO(pSwapChain->GetDesc(&swapChainDesc));
			GFX_THROW_INFO(pSwapChain->ResizeBuffers(frameCount, clientWidth, clientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));
			currentBackBufferIndex = pSwapChain->GetCurrentBackBufferIndex();

			auto rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(pBackBuffersHeap->GetCPUDescriptorHandleForHeapStart());
			for (int i = 0; i < frameCount; i++)
			{
				GFX_THROW_INFO(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffers[i])));
				pDevice->CreateRenderTargetView(pBackBuffers[i].Get(), nullptr, rtvHandle);
				rtvHandle.Offset(1, rtvDescriptorSize);
			}

			D3D12_RESOURCE_DESC uavResourceDesc = {};
			uavResourceDesc.DepthOrArraySize = 1;
			uavResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			uavResourceDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			uavResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			uavResourceDesc.Width = clientWidth;
			uavResourceDesc.Height = clientHeight;
			uavResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			uavResourceDesc.MipLevels = 1;
			uavResourceDesc.SampleDesc.Count = 1;

			D3D12_HEAP_PROPERTIES uavHeapProps = {};
			uavHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			uavHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			uavHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			uavHeapProps.CreationNodeMask = 0;
			uavHeapProps.VisibleNodeMask = 0;
			GFX_THROW_INFO(pDevice->CreateCommittedResource(&uavHeapProps, D3D12_HEAP_FLAG_NONE, &uavResourceDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&pUnorderedAccess)));

			D3D12_DESCRIPTOR_HEAP_DESC uavDescriptorHeapDesc = {};
			uavDescriptorHeapDesc.NumDescriptors = 1;
			uavDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			uavDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			GFX_THROW_INFO(pDevice->CreateDescriptorHeap(&uavDescriptorHeapDesc, IID_PPV_ARGS(&pUAVHeap)));

			D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = pUAVHeap->GetCPUDescriptorHandleForHeapStart();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			pDevice->CreateUnorderedAccessView(pUnorderedAccess.Get(), nullptr, &uavDesc, uavHandle);
		}
	}

	void Graphics::SetRenderConstants()
	{
		RenderConstants constants;
		ZeroMemory(&constants, sizeof(constants));
		constants.time = float(clock()) / CLOCKS_PER_SEC;
		constants.frame = currentFrame;
		constants.width = clientWidth;
		constants.height = clientHeight;

		constants.originX = 3.6f * std::sin(constants.time * 1.5f) + 0.5f;
		constants.originY = 3.0f + std::sin(constants.time * 2.0f);
		constants.originZ = 3.6f * std::cos(constants.time * 1.0f) + 0.5f;

		float mulBy = std::sqrt(constants.originX * constants.originX + constants.originZ * constants.originZ);
		constants.originX /= mulBy * 0.2f;
		constants.originZ /= mulBy * 0.25f;

		constants.padding1 = 0;
		constants.padding2 = 0;
		constants.padding4 = 0;
		constants.padding5 = 0;
		constants.padding6 = 0;

		constants.previousOriginX = (float)camera.position.x;
		constants.previousOriginY = (float)camera.position.y;
		constants.previousOriginZ = (float)camera.position.z;

		constants.previousMatrix[0] = camera.cameraToWorldMatrix.GetGpuMatrix()[0];
		constants.previousMatrix[1] = camera.cameraToWorldMatrix.GetGpuMatrix()[1];
		constants.previousMatrix[2] = camera.cameraToWorldMatrix.GetGpuMatrix()[2];
		constants.previousMatrix[3] = camera.cameraToWorldMatrix.GetGpuMatrix()[3];
		constants.previousMatrix[4] = camera.cameraToWorldMatrix.GetGpuMatrix()[4];
		constants.previousMatrix[5] = camera.cameraToWorldMatrix.GetGpuMatrix()[5];
		constants.previousMatrix[6] = camera.cameraToWorldMatrix.GetGpuMatrix()[6];
		constants.previousMatrix[7] = camera.cameraToWorldMatrix.GetGpuMatrix()[7];
		constants.previousMatrix[8] = camera.cameraToWorldMatrix.GetGpuMatrix()[8];

		camera.position = Vector3(constants.originX, constants.originY, constants.originZ);
		camera.targetPosition = Vector3(0, 0, 0);
		camera.UpdateCameraToWorldMatrix();

		constants.mat[0] = camera.cameraToWorldMatrix.GetGpuMatrix()[0];
		constants.mat[1] = camera.cameraToWorldMatrix.GetGpuMatrix()[1];
		constants.mat[2] = camera.cameraToWorldMatrix.GetGpuMatrix()[2];
		constants.mat[3] = camera.cameraToWorldMatrix.GetGpuMatrix()[3];
		constants.mat[4] = camera.cameraToWorldMatrix.GetGpuMatrix()[4];
		constants.mat[5] = camera.cameraToWorldMatrix.GetGpuMatrix()[5];
		constants.mat[6] = camera.cameraToWorldMatrix.GetGpuMatrix()[6];
		constants.mat[7] = camera.cameraToWorldMatrix.GetGpuMatrix()[7];
		constants.mat[8] = camera.cameraToWorldMatrix.GetGpuMatrix()[8];

		pCommandList->SetComputeRoot32BitConstants(2, (UINT)std::ceil(sizeof(RenderConstants) / 4), &constants, 0);
	}

	void Graphics::QueueCommands()
	{
		HRESULT hr;

		GFX_THROW_INFO(pCommandAllocator->Reset());
		GFX_THROW_INFO(pCommandList->Reset(pCommandAllocator.Get(), pRenderPipelineState.Get()));

		UpdateUIBuffer();
		UpdateTriangleBuffer();

		//Set root signature
		pCommandList->SetComputeRootSignature(pRootSignature.Get());

		//Bind Triangle buffer, Vertex Buffer, Node Hierarchy, Render Texture, and UI buffer
		auto triangleBufferHeap = triangleDescriptorHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &triangleBufferHeap);
		pCommandList->SetComputeRootDescriptorTable(3, triangleBufferHeap->GetGPUDescriptorHandleForHeapStart());

		auto vertexBufferHeap = vertexDescriptorHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &vertexBufferHeap);
		pCommandList->SetComputeRootDescriptorTable(5, vertexBufferHeap->GetGPUDescriptorHandleForHeapStart());

		auto bvhBufferHeap = boundingVolumeHierarchyDescriptorHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &bvhBufferHeap);
		pCommandList->SetComputeRootDescriptorTable(4, bvhBufferHeap->GetGPUDescriptorHandleForHeapStart());

		auto renderTextureHeap = pUAVHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &renderTextureHeap);
		pCommandList->SetComputeRootDescriptorTable(0, pUAVHeap->GetGPUDescriptorHandleForHeapStart());

		auto tempTextureHeap = pTemporaryHistoryBufferHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &tempTextureHeap);
		pCommandList->SetComputeRootDescriptorTable(6, pTemporaryHistoryBufferHeap->GetGPUDescriptorHandleForHeapStart());
		auto reprojectionBufferHeap = pHistoryBufferHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &reprojectionBufferHeap);
		pCommandList->SetComputeRootDescriptorTable(7, pHistoryBufferHeap->GetGPUDescriptorHandleForHeapStart());
		auto geomertyTempTextureHeap = pGeomertyTemporaryHistoryBufferHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &geomertyTempTextureHeap);
		pCommandList->SetComputeRootDescriptorTable(9, pGeomertyTemporaryHistoryBufferHeap->GetGPUDescriptorHandleForHeapStart());
		auto geomertyReprojectionBufferHeap = pGeomertyHistoryBufferHeap.Get();
		pCommandList->SetDescriptorHeaps(1, &geomertyReprojectionBufferHeap);
		pCommandList->SetComputeRootDescriptorTable(8, pGeomertyHistoryBufferHeap->GetGPUDescriptorHandleForHeapStart());

		if (uiManager.ElementCount() != 0)
		{
			auto uiBufferheap = uiElementDescriptorHeap.Get();
			pCommandList->SetDescriptorHeaps(1, &uiBufferheap);
			pCommandList->SetComputeRootDescriptorTable(1, uiElementDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		}

		//Set Constants
		SetRenderConstants();

		//Execute compute shader
		double threadGroupSize = 32;
		pCommandList->Dispatch((UINT)std::ceil((double)clientWidth / threadGroupSize), (UINT)std::ceil((double)clientHeight / threadGroupSize), 1);

		//Copy history buffer to temp buffer and render uav to backbuffer
		D3D12_RESOURCE_BARRIER transitionToCopyBarrier[6] = { CD3DX12_RESOURCE_BARRIER::Transition(pBackBuffers[currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST),
															  CD3DX12_RESOURCE_BARRIER::Transition(pUnorderedAccess.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
															  CD3DX12_RESOURCE_BARRIER::Transition(pHistoryBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE), 
															  CD3DX12_RESOURCE_BARRIER::Transition(pGeomertyHistoryBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
															  CD3DX12_RESOURCE_BARRIER::Transition(pTemporaryHistoryBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST), 
															  CD3DX12_RESOURCE_BARRIER::Transition(pGeomertyTemporaryHistoryBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST) };

		pCommandList->ResourceBarrier(6, transitionToCopyBarrier);

		pCommandList->CopyResource(pBackBuffers[currentBackBufferIndex].Get(), pUnorderedAccess.Get());
		pCommandList->CopyResource(pTemporaryHistoryBuffer.Get(), pHistoryBuffer.Get());
		pCommandList->CopyResource(pGeomertyTemporaryHistoryBuffer.Get(), pGeomertyHistoryBuffer.Get());

		//Reset resource states
		D3D12_RESOURCE_BARRIER resetStateBarrier[6] = { CD3DX12_RESOURCE_BARRIER::Transition(pBackBuffers[currentBackBufferIndex].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON),
														CD3DX12_RESOURCE_BARRIER::Transition(pUnorderedAccess.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
														CD3DX12_RESOURCE_BARRIER::Transition(pHistoryBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
														CD3DX12_RESOURCE_BARRIER::Transition(pGeomertyHistoryBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
														CD3DX12_RESOURCE_BARRIER::Transition(pTemporaryHistoryBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS), 
														CD3DX12_RESOURCE_BARRIER::Transition(pGeomertyTemporaryHistoryBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) };

		pCommandList->ResourceBarrier(6, resetStateBarrier);

		GFX_THROW_INFO(pCommandList->Close());
	}

	void Graphics::Render()
	{
		if (pipelineObjectsInitialized)
		{
			HRESULT hr;

			QueueCommands();

			ID3D12CommandList* ppCommandLists[] = { pCommandList.Get() };
			pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

#ifndef NDEBUG
			GFX_INFO_START();
#endif
			UINT syncInterval = VSyncEnabled ? 1 : 0;
			if (FAILED(hr = pSwapChain->Present(syncInterval, 0)))
			{
				if (hr == DXGI_ERROR_DEVICE_REMOVED)
				{
					throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
				}
				else
				{
					GFX_EXCEPT(hr);
				}
			}

			currentFrame++;

			WaitForPreviousFrame();
		}
	}
}