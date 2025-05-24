#pragma once

const UINT SWAP_CHAIN_FRAME_COUNT = 2;

class CD3D12ResourceManager;
class CDescriptorPool;
class CSimpleConstantBufferPool;
class CSingleDescriptorAllocator;

class CD3D12Renderer
{
	static const UINT MAX_DRAW_COUNT_PER_FRAME = 256;
	static const UINT MAX_DESCRIPTOR_COUNT = 4096;

private:
	HWND m_hWindow = nullptr;
	ID3D12Device5* m_pD3DDevice = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	CD3D12ResourceManager* m_pResourceManager = nullptr;
	CSingleDescriptorAllocator* m_pSingleDescriptorAllocator = nullptr;

	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	CDescriptorPool* m_pDescriptorPool = nullptr;
	CSimpleConstantBufferPool* m_pConstantBufferPool = nullptr;
	UINT64	m_ui64FenceValue = 0;

	D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_ADAPTER_DESC1 m_AdapterDesc = {};
	IDXGISwapChain3* m_pSwapChain = nullptr;
	D3D12_VIEWPORT m_Viewport = {};
	D3D12_RECT m_ScissorRect = {};
	DWORD m_dwWidth = 0;
	DWORD m_dwHeight = 0;

	ID3D12Resource* m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {};
	ID3D12DescriptorHeap* m_pRTVHeap = nullptr;
	ID3D12DescriptorHeap* m_pDSVHeap = nullptr;
	ID3D12DescriptorHeap* m_pSRVHeap = nullptr;
	UINT m_rtvDescriptorSize = 0;
	UINT m_srvDescriptorSize = 0;
	UINT m_dwSwapChainFlags = 0;
	UINT m_uiRenderTargetIndex = 0;
	HANDLE m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;

	DWORD	m_dwCurContextIndex = 0;

public:
	BOOL Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV);
	void BeginRender();
	void EndRender();
	void Present();
	BOOL UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);

private:
	BOOL CreateDescriptorHeapForRTV();
	void CleanupDescriptorHeapForRTV();
	void CreateFence();
	void CleanupFence();
	void CreateCommandList();
	void CleanupCommandList();

	UINT64	Fence();
	void	WaitForFenceValue();

	void CleanUp();

public:

	void* CreateBasicMeshObject();
	void DeleteBasicMeshObject(void* pMeshObjHandle);

	void* CreateTiledTexture(UINT TexWidth, UINT TexHeight, DWORD r, DWORD g, DWORD b);
	void DeleteTexture(void* pTexHandle);

	void RenderMeshObject(void* pMeshObjHandle, float x_offset, float y_offset, void* pTexHandle);

	ID3D12Device5* GetD3DDevice() const { return m_pD3DDevice; }
	CD3D12ResourceManager* GetResourceManager() const { return m_pResourceManager; } // Placeholder for resource manager
	CDescriptorPool* GetDescriptorPool() { return m_pDescriptorPool; }
	CSimpleConstantBufferPool* GetConstantBufferPool() { return m_pConstantBufferPool; }
	UINT GetSrvDescriptorSize() { return m_srvDescriptorSize; }

public:
	CD3D12Renderer();
	~CD3D12Renderer();
};

