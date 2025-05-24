#pragma once

enum BASIC_MESH_DESCRIPTOR_INDEX
{
	BASIC_MESH_DESCRIPTOR_INDEX_CBV = 0,
	BASIC_MESH_DESCRIPTOR_INDEX_TEX = 1,
};

class CD3D12Renderer;
class CBasicMeshObject
{
public:
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 2;	// | Constant Buffer | Tex |

private:
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD m_dwInitRefCount;

	CD3D12Renderer* m_pRenderer = nullptr;

	// Vertex Data
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

public:
	BOOL InitCommonResource();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();

	void Cleanup();

public:
	BOOL Initialize(CD3D12Renderer* _pRenderer);
	void Draw(ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, D3D12_CPU_DESCRIPTOR_HANDLE srv);
	BOOL CreateMesh();

public:
	CBasicMeshObject();
	~CBasicMeshObject();
};