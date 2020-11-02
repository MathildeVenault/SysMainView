#include "Utils.h"


typedef void(__stdcall* __RtlZeroMemory)(
	void* Destination,
	size_t Length
	);
typedef void(__stdcall* __RtlMoveMemory)(
	void* Destination,
	const void* Source,
	size_t      Length
	);

typedef DWORD(__stdcall* __RtlComputeCrc32)(
	DWORD       dwInitial,
	const BYTE* pData,
	INT         iLen
	);
typedef NTSTATUS(__stdcall* __RtlDecompressBufferEx)(
	USHORT CompressionFormat, PUCHAR UncompressedBuffer,
	ULONG UncompressedBufferSize, PUCHAR CompressedBuffer,
	ULONG CompressedBufferSize, PULONG FinalUncompressedSize,
	PVOID  WorkSpace);

typedef WCHAR(__stdcall* __RtlUpcaseUnicodeChar)(
	WCHAR SourceCharacter
	);

typedef NTSTATUS(__stdcall* __RtlGetCompressionWorkSpaceSize)(
	USHORT CompressionFormatAndEngine,
	PULONG CompressBufferWorkSpaceSize,
	PULONG CompressFragmentWorkSpaceSize);

typedef ULONG(__stdcall* __RtlNtStatusToDosError)(
	NTSTATUS Status
	);
typedef NTSTATUS(__stdcall* __RtlCompressBuffer)(
	USHORT CompressionFormatAndEngine,
	PUCHAR UncompressedBuffer,
	ULONG  UncompressedBufferSize,
	PUCHAR CompressedBuffer,
	ULONG  CompressedBufferSize,
	ULONG  UncompressedChunkSize,
	PULONG FinalCompressedSize,
	PVOID  WorkSpace
	);




static HMODULE hNtDll = 0;
static __RtlComputeCrc32 _RtlComputeCrc32 = 0;
static __RtlUpcaseUnicodeChar _RtlUpcaseUnicodeChar = 0;
static __RtlDecompressBufferEx _RtlDecompressBufferEx = 0;
static __RtlCompressBuffer _RtlCompressBuffer = 0;
static __RtlGetCompressionWorkSpaceSize _RtlGetCompressionWorkSpaceSize = 0;
static __RtlNtStatusToDosError _RtlNtStatusToDosError = 0;
static __RtlMoveMemory _RtlMoveMemory = 0;
static __RtlZeroMemory _RtlZeroMemory = 0;


//
//  Calculates the checksum of the data in parameters
//
DWORD CalculChecksum(BYTE* Input, DWORD InputSize)
{
	if (0 == _RtlComputeCrc32)
	{
		_RtlComputeCrc32 = (__RtlComputeCrc32)GetProcAddress(hNtDll, "RtlComputeCrc32");
		if (0 == _RtlComputeCrc32)
			return 0;
	}
	return _RtlComputeCrc32(0, Input, InputSize);


}

VOID TprintfC(_In_ COLOR_TPRINTF Color, LPTSTR Format, ...) {

	va_list arglist;

	va_start(arglist, Format);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color); // Use the expected color.
	_vtcprintf(Format, arglist);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), White); //get back to white police

	va_end(arglist);
}

//
//  Compress the buffer get through parameters with the XPRESS_HUFFMAN
//
UCHAR* BufferCompress(UCHAR* input, ULONG inputSize, UCHAR* output, ULONG outputSize, ULONG* FinaloutputSize)
{
	ULONG dummy = 0, UncompressedChunkSize = 0;
	NTSTATUS Status = 0;
	ULONG WorkSpaceSize = 0, WorkFragSize = 0;
	PVOID WorkSpace = NULL;

	if (0 == hNtDll)
	{
		hNtDll = LoadLibrary(_T("ntdll.dll"));
		if (0 == hNtDll) {
			TprintfC(Red, _T("[-] Error LoadLibrary() %d"), GetLastError());
			return NULL;
		}
	}

	if (0 == _RtlCompressBuffer)
	{
		_RtlCompressBuffer = (__RtlCompressBuffer)GetProcAddress(hNtDll, "RtlCompressBuffer");
		_RtlGetCompressionWorkSpaceSize = (__RtlGetCompressionWorkSpaceSize)GetProcAddress(hNtDll, "RtlGetCompressionWorkSpaceSize");
		_RtlNtStatusToDosError = (__RtlNtStatusToDosError)GetProcAddress(hNtDll, "RtlNtStatusToDosError");
		if (0 == _RtlCompressBuffer || _RtlGetCompressionWorkSpaceSize == 0)
			return NULL;
	}

	Status = _RtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_XPRESS_HUFF, &WorkSpaceSize, &WorkFragSize);
	if (Status != ERROR_SUCCESS) {

		TprintfC(Red, _T("[-] Error:RtlCompressWorkSpaceSize() %d.\n"), _RtlNtStatusToDosError(Status));

		return NULL;
	}

	WorkSpace = (PVOID)malloc(WorkSpaceSize);
	Status = _RtlCompressBuffer(COMPRESSION_FORMAT_XPRESS_HUFF, input, inputSize, output, outputSize, UncompressedChunkSize, &dummy, WorkSpace);

	if (Status != ERROR_SUCCESS) {
		TprintfC(Red, _T("[-] Error:RtlCompressBufferEx() %d.\n"), _RtlNtStatusToDosError(Status));

		return NULL;
	}
	*FinaloutputSize = dummy;
	if (WorkSpace)
	{
		free(WorkSpace);
	}

	if (hNtDll)
	{
		FreeLibrary(hNtDll);
	}



	return output;
}

//
//  Decompress the buffer get through parameters with the XPRESS_HUFFMAN
//
UCHAR* BufferDecompress(UCHAR* input, ULONG inputSize, UCHAR* output, ULONG outputSize)
{
	ULONG dummy = 0;
	NTSTATUS Status = 0;
	ULONG WorkSpaceSize = 0, WorkFragSize = 0;
	PVOID WorkSpace;

	if (0 == hNtDll)
	{
		hNtDll = LoadLibrary(_T("ntdll.dll"));
		if (0 == hNtDll) {
			TprintfC(Red, _T("[-] Error LoadLibrary() %d"), GetLastError());
			return NULL;
		}
	}

	if (0 == _RtlDecompressBufferEx)
	{
		_RtlDecompressBufferEx = (__RtlDecompressBufferEx)GetProcAddress(hNtDll, "RtlDecompressBufferEx");
		_RtlGetCompressionWorkSpaceSize = (__RtlGetCompressionWorkSpaceSize)GetProcAddress(hNtDll, "RtlGetCompressionWorkSpaceSize");
		_RtlNtStatusToDosError = (__RtlNtStatusToDosError)GetProcAddress(hNtDll, "RtlNtStatusToDosError");
		if (0 == _RtlDecompressBufferEx || _RtlGetCompressionWorkSpaceSize == 0)
			return NULL;
	}

	Status = _RtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_XPRESS_HUFF, &WorkSpaceSize, &WorkFragSize);

	if (Status != ERROR_SUCCESS) {
		TprintfC(Red, _T("[-] Error:RtlGetCompressionWorkSpaceSize() %d.\n"), _RtlNtStatusToDosError(Status));
		return NULL;
	}

	WorkSpace = (PVOID)malloc(WorkSpaceSize);

	Status = _RtlDecompressBufferEx(COMPRESSION_FORMAT_XPRESS_HUFF, output, outputSize, input, inputSize, &dummy, WorkSpace);
	if (Status != ERROR_SUCCESS) {
		TprintfC(Red, _T("[-] Error:RtlCompressBufferEx() %d.\n"), _RtlNtStatusToDosError(Status));
		return NULL;
	}

	if (WorkSpace)
	{
		free(WorkSpace);
	}

	if (hNtDll)
	{
		FreeLibrary(hNtDll);
	}

	return output;
}

//
// Hash a command line within the algorithm user by Sysmain on Windows 10
//
ULONG64 HashCmd() {

	TCHAR cmd[500] = { '\0' };
	ULONG64 i = 0;
	ULONG64 HashValue_ = 0;
	SIZE_T NameLength = 0;
	WCHAR c = 0;

	TprintfC(Blue, _T("\n-------------------------\n"));
	TprintfC(Blue, _T("---------- HASH ---------\n"));
	TprintfC(Blue, _T("-------------------------\n\n"));


	if (hNtDll == 0) {
		hNtDll = LoadLibrary(_T("ntdll.dll"));

		if (hNtDll == 0) {
			TprintfC(Red, _T(" [-] Error: Loading library ntdll ; %d. \n"), GetLastError());
			return 0;
		}
	}
	if (hNtDll != 0 && _RtlUpcaseUnicodeChar == 0) {
		_RtlUpcaseUnicodeChar = (__RtlUpcaseUnicodeChar)GetProcAddress(hNtDll, "RtlUpcaseUnicodeChar");
		if (_RtlUpcaseUnicodeChar == 0) {
			TprintfC(Red, _T(" [-] Error: Loading RtlUpcaseUnicodeChar ;%d.\n"), GetLastError());
			return 0;
		}
	}

	// Get the value to hash
	_tprintf(_T("Please Enter the string to hash : "));
	_tscanf_s(_T("%s"), (LPTSTR)cmd, (unsigned int)_countof(cmd));
	NameLength = _tcslen(cmd);
	if (NameLength == 0) {
		return 0;
	}

	TprintfC(Green, _T("[+] %s will be hashed.\n"), (LPTSTR)cmd);

	//Hash algorithm
	HashValue_ = 314159;
	for (i = 0; i < NameLength; i++) {
		c = cmd[i];
		c = _RtlUpcaseUnicodeChar(c);
		HashValue_ = (HashValue_ * 37 + c) * 37;
	}
	TprintfC(Green, _T(" [+] Your hash value : %08X.\n"), HashValue_);


	return HashValue_;
}


_Result_nullonfailure_
LPTSTR CopyBuffer(_In_ LPTSTR Buffer, _In_opt_ SIZE_T Size) {

	LPTSTR Output = NULL;

	if (0 == hNtDll)
	{
		hNtDll = LoadLibrary(_T("ntdll.dll"));
		if (0 == hNtDll) {
			TprintfC(Red, _T("[-] Error LoadLibrary() %d"), GetLastError());
			return NULL;
		}
	}

	if (0 == _RtlMoveMemory)
	{
		_RtlMoveMemory = (__RtlMoveMemory)GetProcAddress(hNtDll, "RtlMoveMemory");
		if (0 == _RtlMoveMemory)
		{
			TprintfC(Red, _T("[-] Error RtlMoveMemory() %d"), GetLastError());
			return NULL;
		}
	}

	if (Size == 0) {
		Size = _tcslen(Buffer);
	}

	Output = (LPTSTR)calloc(Size + 1, sizeof(TCHAR));
	if (Output == NULL) {
		return NULL;
	}

	_RtlMoveMemory(Output, Buffer, Size * sizeof(TCHAR));

	return Output;
}

_Result_nullonfailure_
LPTSTR SelectTargetFileByExplorer(VOID) {

	OPENFILENAME ofn;					// common dialog box structure
	TCHAR szFile[MAX_PATH] = { 0 };		// if using TCHAR macros
	LPTSTR Output = NULL;

	// Initialize OPENFILENAME


	/*
	if (0 == _RtlZeroMemory)
	{
		_RtlZeroMemory = (__RtlZeroMemory)GetProcAddress(hNtDll, "RtlZeroMemory");
		if (0 == _RtlZeroMemory)
		{
			TprintfC(Red, _T("[-] Error RtlZeroMemory() %d"), GetLastError());
			return NULL;
		}
	}
	_RtlZeroMemory(&ofn, sizeof(OPENFILENAME));*/
	if (SecureZeroMemory(&ofn, sizeof(OPENFILENAME)) == NULL)
	{
		TprintfC(Red, _T("[-] Error SecureZeroMemory() %d"), GetLastError());
		return NULL;
	}

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = _T('\0');
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = _T("All\0*.*\0Text\0*.pf\0*.db\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn) == TRUE) {
		TprintfC(Green, _T("\n[+] Selected file: \"%s\".\n"), ofn.lpstrFile);
	}
	else {
		TprintfC(Red, _T("[-] Error: GetOpenFileName has failed %d.\n"), GetLastError());
	}

	Output = CopyBuffer(ofn.lpstrFile, 0);
	return Output;
}