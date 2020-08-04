#pragma once

#ifndef GETINFO_H
#define GETINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include "Utils.h"

typedef struct DbBuffer_s
{
	DWORD MagicNumber;
	DWORD TotalSize;
	DWORD HeaderSize;
	DWORD FileType;
	DWORD Params[8];
	DWORD Reserved;
	DWORD CountVolumes;
	DWORD CountEntries;
	DWORD Reserved_Condition;
	DWORD Reserved_Condition2;

}BUFFER_DB;


typedef struct PfBuffer_s
{
	DWORD OS_id;
	DWORD SCCA;
	DWORD FormatCondition;
	DWORD TotalSize;
	BYTE Name[60];
	DWORD HashValue;
	DWORD Reserved[2];
	DWORD CountEntries;
	DWORD Reserved2[2];
	DWORD OffsetEntry;
	DWORD SizeEntry;
	DWORD OffsetVolume;
	DWORD CountVolume;
	DWORD Reserved3[3];
	FILETIME Time[8];
	DWORD Reserved4[2];
	WORD CountRun;


}BUFFER_PF;

void* ReadHeader(LPTSTR lpFileName);
void GetInformation_Pf(LPTSTR lpFileName);
void GetInformation_Db(LPTSTR lpFileName);
void AgAppLaunch(BYTE* Data);

#endif // !1

