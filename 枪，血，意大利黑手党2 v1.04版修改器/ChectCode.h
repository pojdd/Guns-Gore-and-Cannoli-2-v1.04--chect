#pragma once
#include <Windows.h>
class ChectCode
{
public:
	ChectCode();
	~ChectCode();
	void init(int len ,char * objcode, char * activeCode, DWORD start, DWORD end,int offset);
	int len=0;
	char * objcode = 0;
	char * activeCode = 0;
	DWORD start;
	DWORD end;
	int offset;
	DWORD address=0;
	bool isActive=0;
};

