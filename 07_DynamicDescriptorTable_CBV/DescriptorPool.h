#pragma once

class CDescriptorPool
{
	ID3D12Device5* m_pD3DDevice = nullptr;
	UINT m_AllocatedDescriptorCount = 0;
	UINT m_MaxDescriptorCount = 0;
	UINT m_srvDescriptorSize = 0;
	ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptorHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuDescriptorHandle = {};

	

public:
	BOOL Initialize(ID3D12Device5* pD3DDevice, UINT MaxDescriptorCount);
	BOOL AllocDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGPUDescriptor, UINT DescriptorCount);
	void Reset();



public:
	ID3D12DescriptorHeap* GetDescriptorHeap() { return m_pDescriptorHeap; }

private:
	void Cleanup();

public:
	CDescriptorPool();
	~CDescriptorPool();

};