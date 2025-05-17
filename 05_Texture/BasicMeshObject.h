#pragma once

enum BASIC_MESH_DESCRIPTOR_INDEX
{
	BASIC_MESH_DESCRIPTOR_INDEX_TEX = 0
};

class CD3D12Renderer;
class CBasicMeshObject
{
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 1;	// | Tex 

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

	// texture
	ID3D12Resource* m_pTexResource = nullptr;

	// for Descriptor Table
	UINT	m_srvDescriptorSize = 0;
	ID3D12DescriptorHeap* m_pDescritorHeap = nullptr;	// Physical Resource as Descriptor Table

public:
	BOOL InitCommonResource();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();

	BOOL CreateDescriptorTable();

	void Cleanup();

public:
	BOOL Initialize(CD3D12Renderer* _pRenderer);
	void Draw(ID3D12GraphicsCommandList* pCommandList);
	BOOL CreateMesh();

public:
	CBasicMeshObject();
	~CBasicMeshObject();
};