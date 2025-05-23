#include "pch.h"
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dx12.h>
#include "D3DUtil.h"

void SetDebugLayerInfo(ID3D12Device* _pD3DDevice)
{
	ID3D12InfoQueue* pInfoQueue = nullptr;
	_pD3DDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
	if (pInfoQueue)
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
			// Workarounds for debug layer issues on hybrid-graphics systems
			D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = (UINT)_countof(hide);
		filter.DenyList.pIDList = hide;
		pInfoQueue->AddStorageFilterEntries(&filter);

		pInfoQueue->Release();
		pInfoQueue = nullptr;
	}
}

void SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamplerDesc, UINT RegisterIndex)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//pOutSamperDesc->Filter = D3D12_FILTER_ANISOTROPIC;
	pOutSamplerDesc->Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	pOutSamplerDesc->AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamplerDesc->AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamplerDesc->AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamplerDesc->MipLODBias = 0.0f;
	pOutSamplerDesc->MaxAnisotropy = 16;
	pOutSamplerDesc->ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	pOutSamplerDesc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pOutSamplerDesc->MinLOD = -FLT_MAX;
	pOutSamplerDesc->MaxLOD = D3D12_FLOAT32_MAX;
	pOutSamplerDesc->ShaderRegister = RegisterIndex;
	pOutSamplerDesc->RegisterSpace = 0;
	pOutSamplerDesc->ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}