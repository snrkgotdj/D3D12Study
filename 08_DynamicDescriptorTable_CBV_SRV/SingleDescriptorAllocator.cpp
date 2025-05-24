
#include "pch.h"
#include <d3d12.h>
#include <d3dx12.h>

#include "SingleDescriptorAllocator.h"

CSingleDescriptorAllocator::CSingleDescriptorAllocator()
{

}

CSingleDescriptorAllocator::~CSingleDescriptorAllocator()
{
	Cleanup();
}

BOOL CSingleDescriptorAllocator::Initialize(ID3D12Device* pDevice, DWORD dwMaxCount, D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
{
	m_pD3DDevice = pDevice;
	m_pD3DDevice->AddRef();

	D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
	commonHeapDesc.NumDescriptors = dwMaxCount;
	commonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	commonHeapDesc.Flags = Flags;

	HRESULT hrCreateDescriptorHeap = m_pD3DDevice->CreateDescriptorHeap(&commonHeapDesc, IID_PPV_ARGS(&m_pHeap));
	if (FAILED(hrCreateDescriptorHeap))
	{
		__debugbreak();
	}

	m_IndexCreator.Initialize(dwMaxCount);

	m_DescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return TRUE;
}

void CSingleDescriptorAllocator::Cleanup()
{
	if (m_pHeap)
	{
		m_pHeap->Release();
		m_pHeap = nullptr;
	}

	if (m_pD3DDevice)
	{
		m_pD3DDevice->Release();
		m_pD3DDevice = nullptr;
	}
}

BOOL CSingleDescriptorAllocator::AllocDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUHandle)
{
	DWORD dwIndex = m_IndexCreator.Alloc();
	if (dwIndex == -1)
	{
		return FALSE;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHandle(m_pHeap->GetCPUDescriptorHandleForHeapStart(), dwIndex, m_DescriptorSize);
	*pOutCPUHandle = DescriptorHandle;

	return TRUE;
}

void CSingleDescriptorAllocator::FreeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle)
{
	D3D12_CPU_DESCRIPTOR_HANDLE base = m_pHeap->GetCPUDescriptorHandleForHeapStart();
#ifdef _DEBUG
	if (base.ptr > DescriptorHandle.ptr)
	{
		__debugbreak();
	}
#endif

	DWORD dwIndex = (DWORD)(DescriptorHandle.ptr - base.ptr) / m_DescriptorSize;
	m_IndexCreator.Free(dwIndex);
}