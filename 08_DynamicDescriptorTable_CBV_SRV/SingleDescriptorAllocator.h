#pragma once

#include "Util/IndexCreator.h"

class CSingleDescriptorAllocator
{
private:
	ID3D12Device* m_pD3DDevice = nullptr;
	ID3D12DescriptorHeap* m_pHeap = nullptr;
	CIndexCreator m_IndexCreator;
	UINT m_DescriptorSize = 0;

public:
	BOOL Initialize(ID3D12Device*  pDevice, DWORD dwMaxCount, D3D12_DESCRIPTOR_HEAP_FLAGS Flags);

private:
	void Cleanup();

public:
	BOOL AllocDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUHandle);
	void FreeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle);

public:
	CSingleDescriptorAllocator();
	~CSingleDescriptorAllocator();

};