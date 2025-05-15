
#include "pch.h"

#include "D3D12ResourceManager.h"

CD3D12ResourceManager::CD3D12ResourceManager()
{

}

CD3D12ResourceManager::~CD3D12ResourceManager()
{
	Cleanup();
}

BOOL CD3D12ResourceManager::Initialize(ID3D12Device5* _pD3DDevice)
{
	m_pD3DDevice = _pD3DDevice;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(m_pD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue))))
	{
		__debugbreak();
		return FALSE;
	}

	CreateCommandList();

	CreateFence();

	return TRUE;
}

void CD3D12ResourceManager::CreateCommandList()
{

	HRESULT hr = m_pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));
	if(FAILED(hr))
	{
		__debugbreak();
		return;
	}

	HRESULT hrCreateCommandList = m_pD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator, nullptr, IID_PPV_ARGS(&m_pCommandList));
	if (FAILED(hrCreateCommandList))
	{
		__debugbreak();
		return;
	}
	
	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	m_pCommandList->Close();
}

void CD3D12ResourceManager::CleanupCommandList()
{
	if (m_pCommandAllocator)
	{
		m_pCommandAllocator->Release();
		m_pCommandAllocator = nullptr;
	}

	if (m_pCommandList)
	{
		m_pCommandList->Release();
		m_pCommandList = nullptr;
	}
}

void CD3D12ResourceManager::CreateFence()
{
	HRESULT hrCreateFence = m_pD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(hrCreateFence))
	{
		__debugbreak();
		return;
	}

	m_ui64FenceValue = 0;

	// Create an event handle to use for frame synchronization.
	m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void CD3D12ResourceManager::CleanupFence()
{
	if(m_hFenceEvent)
	{
		CloseHandle(m_hFenceEvent);
		m_hFenceEvent = nullptr;
	}

	if (m_pFence)
	{
		m_pFence->Release();
		m_pFence = nullptr;
	}
}

void CD3D12ResourceManager::Cleanup()
{
	WaitForFenceValue();

	if (m_pCommandQueue)
	{
		m_pCommandQueue->Release();
		m_pCommandQueue = nullptr;
	}

	CleanupCommandList();
	CleanupFence();
}

UINT64 CD3D12ResourceManager::Fence()
{
	m_ui64FenceValue += 1;
	m_pCommandQueue->Signal(m_pFence, m_ui64FenceValue);
	return m_ui64FenceValue;
}

void CD3D12ResourceManager::WaitForFenceValue()
{
	const UINT64 ExpectedFenceValue = m_ui64FenceValue;

	// Wait until the previous frame is finished.
	if (m_pFence->GetCompletedValue() < ExpectedFenceValue)
	{
		m_pFence->SetEventOnCompletion(ExpectedFenceValue, m_hFenceEvent);
		WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

HRESULT CD3D12ResourceManager::CreateVertexBuffer(UINT _SizePerVertex, DWORD _dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* _pOutVertexBufferView, ID3D12Resource** _ppOutBuffer, void* pInitData)
{
	HRESULT hr = S_OK;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	UINT vertexBufferSize = _SizePerVertex * _dwVertexNum;

	hr = m_pD3DDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&pVertexBuffer));

	if (FAILED(hr))
	{
		__debugbreak();
		goto lb_return;
	}

	if (pInitData)
	{
		if (FAILED(m_pCommandAllocator->Reset()))
		{
			__debugbreak();
		}
			
		if(FAILED(m_pCommandList->Reset(m_pCommandAllocator, nullptr)))
		{
			__debugbreak();
		}

		hr = m_pD3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer));

		if (FAILED(hr))
		{
			__debugbreak();
			goto lb_return;
		}

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin = nullptr;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.

		hr = pUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		if (FAILED(hr))
		{
			__debugbreak();
			goto lb_return;
		}
		memcpy(pVertexDataBegin, pInitData, vertexBufferSize);
		pUploadBuffer->Unmap(0, nullptr);

		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		m_pCommandList->CopyBufferRegion(pVertexBuffer, 0, pUploadBuffer, 0, vertexBufferSize);
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		m_pCommandList->Close();

		ID3D12CommandList* ppCommandLists[] = { m_pCommandList };
		m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		Fence();
		WaitForFenceValue();
	}

	// Initialize the vertex buffer view.
	vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = _SizePerVertex;
	vertexBufferView.SizeInBytes = vertexBufferSize;

	*_pOutVertexBufferView = vertexBufferView;
	*_ppOutBuffer = pVertexBuffer;

lb_return:
	if (pUploadBuffer)
	{
		pUploadBuffer->Release();
		pUploadBuffer = nullptr;
	}
	return hr;
}