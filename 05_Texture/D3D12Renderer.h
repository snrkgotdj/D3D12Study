#pragma once

const UINT SWAP_CHAIN_FRAME_COUNT = 2;

class CD3D12ResourceManager;

class CD3D12Renderer
{
private:
	HWND m_hWindow = nullptr;
	ID3D12Device5* m_pD3DDevice = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	CD3D12ResourceManager* m_pResourceManager = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
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
	BOOL CreateDescriptorHeap();
	void CleanupDescriptorHeap();
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
	void RenderMeshObject(void* _pMeshObjHandle);

	ID3D12Device5* GetD3DDevice() const { return m_pD3DDevice; }
	CD3D12ResourceManager* GetResourceManager() const { return m_pResourceManager; } // Placeholder for resource manager

public:
	CD3D12Renderer();
	~CD3D12Renderer();
};

