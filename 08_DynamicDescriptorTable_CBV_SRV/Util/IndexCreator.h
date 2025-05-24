#pragma once

class CIndexCreator
{
private:
	DWORD* m_pdwIndexTable = nullptr;
	DWORD m_dwMaxNum = 0;
	DWORD m_dwAllocatedCount = 0;

public:
	BOOL Initialize(DWORD dwNum);

private:
	void Cleanup();

public:
	DWORD Alloc();
	void Free(DWORD dwIndex);
	void Check();

public:
	CIndexCreator();
	~CIndexCreator();
};