#pragma once

class CD3D12ResourceManager
{
private:
	ID3D12Device5* m_pD3DDevice = nullptr;

	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;

	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;
	UINT64	m_ui64FenceValue = 0;

public:
	BOOL Initialize(ID3D12Device5* _pD3DDevice);

private:
	void CreateCommandList();
	void CleanupCommandList();

	void CreateFence();
	void CleanupFence();

	void Cleanup();

private:
	UINT64 Fence();
	void WaitForFenceValue();

public:
	HRESULT CreateVertexBuffer(UINT _SizePerVertex, DWORD _dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* _pOutVertexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);
	HRESULT CreateIndexBuffer(DWORD _dwIndexNum, D3D12_INDEX_BUFFER_VIEW* _pOutIndexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);

public:
	CD3D12ResourceManager();
	~CD3D12ResourceManager();
};