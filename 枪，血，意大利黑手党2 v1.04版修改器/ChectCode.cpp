#include "ChectCode.h"



ChectCode::ChectCode()
{
}


ChectCode::~ChectCode()
{
}

void ChectCode::init(int len, char * objcode, char * activeCode, DWORD start, DWORD end, int offset)
{
	this->len = len;
	this->objcode = objcode;
	this->activeCode = activeCode;
	this->start = start;
	this->end = end;
	this->offset = offset;
}
