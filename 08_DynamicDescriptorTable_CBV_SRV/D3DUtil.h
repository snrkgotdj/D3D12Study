#pragma once

void SetDebugLayerInfo(ID3D12Device* _pD3DDevice);

void SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamplerDesc, UINT RegisterIndex);

inline size_t AlignConstantBufferSize(size_t size)
{
	size_t aligned_size = (size + 255) & (~255);
	return aligned_size;
}