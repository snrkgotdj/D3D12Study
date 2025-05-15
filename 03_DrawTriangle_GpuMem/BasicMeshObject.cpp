#include "pch.h"
#include "typedef.h"
#include <d3dx12.h>
#include <d3dcompiler.h>
#include "D3DUtil.h"
#include "D3D12Renderer.h"

#include "BasicMeshObject.h"

#include "D3D12ResourceManager.h"

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

void CBasicMeshObject::Draw(ID3D12GraphicsCommandList* pCommandList)
{
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);

	pCommandList->SetPipelineState(m_pPipelineState);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
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
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT VertexBufferSize = sizeof(Vertices);

	HRESULT hr = pResourceManager->CreateVertexBuffer(sizeof(BasicVertex), (DWORD)_countof(Vertices), &m_VertexBufferView, &m_pVertexBuffer, Vertices);
	if (FAILED(hr))
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

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	HRESULT hSerializeRootSignature = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError);
	if (FAILED(hSerializeRootSignature))
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