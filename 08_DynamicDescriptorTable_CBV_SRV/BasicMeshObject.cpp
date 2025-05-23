#include "pch.h"
#include "typedef.h"
#include <d3dx12.h>
#include <d3dcompiler.h>
#include "D3DUtil.h"
#include "D3D12Renderer.h"

#include "DescriptorPool.h"
#include "SimpleConstantBufferPool.h"
#include "D3D12ResourceManager.h"

#include "BasicMeshObject.h"

ID3D12RootSignature* CBasicMeshObject::m_pRootSignature = nullptr;
ID3D12PipelineState* CBasicMeshObject::m_pPipelineState = nullptr;
DWORD CBasicMeshObject::m_dwInitRefCount = 0;

CBasicMeshObject::CBasicMeshObject()
{

}

CBasicMeshObject::~CBasicMeshObject()
{
	Cleanup();
}

BOOL CBasicMeshObject::Initialize(CD3D12Renderer* _pRenderer)
{
	m_pRenderer = _pRenderer;

	BOOL bResult = InitCommonResource();
	return bResult;
}

void CBasicMeshObject::Draw(ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos)
{
	// 각각의 draw()작업의 무결성을 보장하려면 draw() 작업마다 다른 영역의 descriptor table(shader visible)과 다른 영역의 CBV를 사용해야 한다.
	// 따라서 draw()할 때마다 CBV는 ConstantBuffer Pool로부터 할당받고, 렌더리용 descriptor table(shader visible)은 descriptor pool로부터 할당 받는다.

	ID3D12Device5* pDevice = m_pRenderer->GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->GetSrvDescriptorSize();
	CDescriptorPool* pDescriptorPool = m_pRenderer->GetDescriptorPool();
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	CSimpleConstantBufferPool* pConstantBufferPool = m_pRenderer->GetConstantBufferPool();

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorTable = {};

	BOOL bAllocDescriptorSuccess = pDescriptorPool->AllocDescriptorTable(&cpuDescriptorTable, &gpuDescriptorTable, DESCRIPTOR_COUNT_FOR_DRAW);
	if (bAllocDescriptorSuccess == false)
	{
		__debugbreak();
	}

	CB_CONTAINER* pCB = pConstantBufferPool->Alloc();
	if (pCB == nullptr)
	{
		__debugbreak();
	}
	CONSTANT_BUFFER_DEFAULT* pConstantBufferDefault = (CONSTANT_BUFFER_DEFAULT*)pCB->pSystemMemAddr;

	// constant buffer의 내용을 설정
	pConstantBufferDefault->offset.x = pPos->x;
	pConstantBufferDefault->offset.y = pPos->y;

	pCommandList->SetGraphicsRootSignature(m_pRootSignature);

	pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

	// 이번에 사용할 constant buffer의 descriptor를 렌더링용(shader visible) descriptor table에 카피
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(cpuDescriptorTable, BASIC_MESH_DESCRIPTOR_INDEX_CBV, srvDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, cbvDest, pCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);	// cpu측 코드에서는 cpu descriptor handle에만 write가능

	pCommandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorTable);

	pCommandList->SetPipelineState(m_pPipelineState);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);

	//pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	pCommandList->DrawInstanced(3, 1, 0, 0);
}

BOOL CBasicMeshObject::CreateMesh()
{
	// 바깥에서 버텍스데이터와 텍스처를 입력하는 식으로 변경할 것

	ID3D12Device5* pD3DDevice = m_pRenderer->GetD3DDevice();
	CD3D12ResourceManager* pResourceManager = m_pRenderer->GetResourceManager();

	// Create the vertex buffer.
	// Define the geometry for a triangle.
	BasicVertex Vertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, 0.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
	};

	const UINT VertexBufferSize = sizeof(Vertices);

	HRESULT hrCreateVertexBuffer = pResourceManager->CreateVertexBuffer(sizeof(BasicVertex), (DWORD)_countof(Vertices), &m_VertexBufferView, &m_pVertexBuffer, Vertices);
	if (FAILED(hrCreateVertexBuffer))
	{
		__debugbreak();
		return FALSE;
	}

	return TRUE;
}

BOOL CBasicMeshObject::InitCommonResource()
{
	if (m_dwInitRefCount > 0)
	{
		m_dwInitRefCount += 1;
		return m_dwInitRefCount;
	}

	InitRootSignature();
	InitPipelineState();

	m_dwInitRefCount += 1;
	return m_dwInitRefCount;
}

void CBasicMeshObject::CleanupSharedResources()
{
	if (m_dwInitRefCount == 0)
	{
		return;
	}
	
	m_dwInitRefCount -= 1;
	if (m_dwInitRefCount == 0)
	{
		if (m_pRootSignature)
		{
			m_pRootSignature->Release();
			m_pRootSignature = nullptr;
		}
		if (m_pPipelineState)
		{
			m_pPipelineState->Release();
			m_pPipelineState = nullptr;
		}
	}
}

BOOL CBasicMeshObject::InitRootSignature()
{
	ID3D12Device5* pD3DDevice = m_pRenderer->GetD3DDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE ranges[1] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Constant Buffer View

	CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	SetDefaultSamplerDesc(&sampler, 0);
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	HRESULT hrSerializeRootSignature = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
	if (FAILED(hrSerializeRootSignature))
	{
		__debugbreak();
	}

	HRESULT hCreateRootSignature = pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
	if (FAILED(hCreateRootSignature))
	{
		__debugbreak();
	}

	if (pSignature)
	{
		pSignature->Release();
		pSignature = nullptr;
	}

	if (pError)
	{
		pError->Release();
		pError = nullptr;
	}

	return TRUE;
}

BOOL CBasicMeshObject::InitPipelineState()
{
	ID3D12Device5* pD3DDevice = m_pRenderer->GetD3DDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	HRESULT hCompileFromFileVS = D3DCompileFromFile(L"./Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, nullptr);
	if (FAILED(hCompileFromFileVS))
	{
		__debugbreak();
	}

	HRESULT hCompileFromFilePS = D3DCompileFromFile(L"./Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, nullptr);
	if (FAILED(hCompileFromFileVS))
	{
		__debugbreak();
	}

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_pRootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	HRESULT hCreateGraphicsPipelineState = pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
	if (FAILED(hCreateGraphicsPipelineState))
	{
		__debugbreak();
	}

	if (pVertexShader)
	{
		pVertexShader->Release();
		pVertexShader = nullptr;
	}
	if (pPixelShader)
	{
		pPixelShader->Release();
		pPixelShader = nullptr;
	}

	return TRUE;
}

void CBasicMeshObject::Cleanup()
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	CleanupSharedResources();
}