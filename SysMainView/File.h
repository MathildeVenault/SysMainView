#pragma once

#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include "GetInfo.h"
#include "Utils.h"


void CompressFile(LPTSTR lpFileName);
void DecompressFile(LPTSTR lpFileName);
void PrintBuffer(BYTE* Input, DWORD InputSize, DWORD NbLines, DWORD From);
DWORD Addat(DWORD Offset, BYTE** Input, DWORD* InputSize, DWORD* ToAdd);
DWORD IsDbFile(LPTSTR FileToD);
BOOL IsFileCompressed(BYTE* Data);
void ModifyFile(LPTSTR INFileName);
DWORD ModifyHours(BYTE* DataDecomp, DWORD offset);

LPTSTR CopyBuffer(_In_ LPTSTR Buffer, _In_opt_ SIZE_T Size);
LPTSTR SelectTargetFileByExplorer(VOID);

#endif // !FILE_H
