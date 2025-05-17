#pragma once

class CD3D12Renderer;
class CBasicMeshObject
{
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;

	// Vertex Data
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	// index data
	ID3D12Resource* m_pIndexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView = {};

public:
	BOOL InitCommonResource();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();

	void Cleanup();

public:
	BOOL Initialize(CD3D12Renderer* _pRenderer);
	void Draw(ID3D12GraphicsCommandList* pCommandList);
	BOOL CreateMesh();

public:
	CBasicMeshObject();
	~CBasicMeshObject();
};