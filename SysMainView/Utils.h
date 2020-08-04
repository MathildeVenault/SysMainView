#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <conio.h>
#include <stdarg.h>
#include <commdlg.h>
#include <WinBase.h>
#include <libloaderapi.h>


typedef enum _COLOR_TPRINTF {

    Red = 12,
    Blue = 11,
    Green = 10,
    Magenta = 5,
    White = 15,
    Yellow = 14

} COLOR_TPRINTF;

VOID TprintfC(_In_ COLOR_TPRINTF Color, LPTSTR Format, ...);
DWORD CalculChecksum(BYTE* Input, DWORD InputSize);
UCHAR* BufferDecompress(UCHAR* input, ULONG inputSize, UCHAR* output, ULONG outputSize);
UCHAR* BufferCompress(UCHAR* input, ULONG inputSize, UCHAR* output, ULONG outputSize, ULONG* FinaloutputSize);
ULONG64 HashCmd();

#endif // ! UTILS_H

