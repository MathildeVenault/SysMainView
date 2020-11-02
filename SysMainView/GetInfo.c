#include "GetInfo.h"
#include "File.h"
void* ReadHeader(LPTSTR lpFileName)
{
	HANDLE hFile = NULL;
	BUFFER_DB* DataDb = NULL;
	BUFFER_PF* DataPf = NULL;
	DWORD DataSize = 0;
	void* Result = NULL;

	__try {

		//Open and read the file
		hFile = CreateFile(lpFileName, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile(%s) %d.\n"), lpFileName, GetLastError());
			__leave;
		}

		DataSize = GetFileSize(hFile, NULL);
		if (DataSize == INVALID_FILE_SIZE) {
			TprintfC(Red, _T("[-] Error: GetFileSize() %d.\n"), GetLastError());
			__leave;
		}

		switch (IsDbFile(lpFileName))
		{
		case 0:
			DataPf = (BUFFER_PF*)calloc(1, sizeof(BUFFER_PF));
			//Intialize the PF structure
			if (ReadFile(hFile, DataPf, sizeof(BUFFER_PF), &DataSize, NULL) == 0) {
				TprintfC(Red, _T("[-] Error: ReadFile(%s) %d for pf buffer.\n"), lpFileName, GetLastError());
				__leave;
			}
			if (DataPf->OS_id == 72171853)
			{
				TprintfC(Red, _T("[-]File compressed; please decompress it first.\n"));
				__leave;
			}
			Result = DataPf;
			break;

		case 1:
			DataDb = (BUFFER_DB*)calloc(1, sizeof(BUFFER_DB));
			//initialize the DB structure
			if (ReadFile(hFile, DataDb, sizeof(BUFFER_DB), &DataSize, NULL) == 0) {
				TprintfC(Red, _T("[-] Error: ReadFile(%s) %d for db buffer.\n"), lpFileName, GetLastError());
				__leave;
			}
			if (DataDb->MagicNumber == 2219655501)
			{
				TprintfC(Red, _T("[-]File compressed; please decompress it first.\n"));
				__leave;
			}
			Result = DataDb;
			break;

		default:
			__leave;
		}
	}
	__finally
	{
		if (hFile)
		{
			CloseHandle(hFile);
		}
		return Result;
	}


}


void GetInformation_Db(LPTSTR lpFileName)
{
	HANDLE hFile = NULL;
	BYTE* Data = NULL;
	DWORD DataSize = 0;
	int ErrCode = 0;

	//Reading information var
	DWORD i = 0;
	WORD* VolumeName = NULL;
	WORD* PathR = NULL;
	size_t PathRSize = 0;
	DWORD offset = 0;
	BUFFER_DB* DataDb = NULL;

	TprintfC(Blue, _T("\n-------------------------\n"));
	TprintfC(Blue, _T("------ INFORMATION ------\n"));
	TprintfC(Blue, _T("-------------------------\n\n"));



	__try {

		//Read and check the header
		DataDb = ReadHeader(lpFileName);
		if (DataDb == NULL)
		{
			__leave;
		}
		//Open and read the all file
		hFile = CreateFile(lpFileName, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile(%s) %d.\n"), lpFileName, GetLastError());
			__leave;
		}


		DataSize = GetFileSize(hFile, NULL);
		if (DataSize == INVALID_FILE_SIZE) {
			TprintfC(Red, _T("[-] Error: GetFileSize() %d.\n"), GetLastError());
			__leave;
		}


		Data = (BYTE*)calloc(DataSize, sizeof(BYTE));
		if (Data == NULL)
		{
			TprintfC(Red, _T("[-] Error: calloc() %d.\n"), GetLastError());
			__leave;
		}


		if (ReadFile(hFile, Data, DataSize, &DataSize, 0) == 0) {
			TprintfC(Red, _T("[-]Error: ReadFile(%s) %d.\n"), lpFileName, GetLastError());
			__leave;
		}


		//other file format 
		if (_tcsstr((LPCTSTR)lpFileName, _T("AgAppLaunch")) != NULL)
		{
			AgAppLaunch(Data);
			__leave;
		}


		TprintfC(Green, _T("[+] File viewed: %s.\n\n"), lpFileName);


		//Display information 
		TprintfC(Magenta, _T("\nMagicNumber:\t\t"));
		_tprintf(_T("%01X \n"), DataDb->MagicNumber);

		TprintfC(Magenta, _T("Totale Size:\t\t"));
		_tprintf(_T("%04X - %d bytes  \n"), DataDb->TotalSize, DataDb->TotalSize);

		TprintfC(Magenta, _T("Header Size:\t\t"));
		_tprintf(_T("%04X - %d bytes  \n"), DataDb->HeaderSize, DataDb->HeaderSize);
		TprintfC(Magenta, _T("FileTypeParams:\t\t"));

		_tprintf(_T("%01X \n"), DataDb->FileType);
		TprintfC(Magenta, _T("Params:\t\t\t"));


		for (i = 0; i < 8; i++)
		{
			_tprintf(_T("%01X "), DataDb->Params[i]);
		}


		TprintfC(Magenta, _T("\nCount of entries:"));
		_tprintf(_T("\t%d \n"), DataDb->CountEntries);

		offset = DataDb->HeaderSize + DataDb->Params[0];// give the offset of the first string
		TprintfC(Magenta, _T("Entries:\n"));

		while (offset < DataSize - 10)
		{
			PathR = (WORD*)(Data + offset);
			PathRSize = _tcsclen(PathR);

			if (PathRSize > 10)
			{
				if (_tcsstr((LPCTSTR)PathR, _T("\\")) != NULL)
				{
					_tprintf(_T("\t\t\t%s \n"), _tcsstr((LPTSTR)PathR, _T("\\")));
					offset += (DWORD)PathRSize * 2 + 1;
				}
			}
			offset++;

		}

	}
	__finally
	{
		if (hFile) {
			CloseHandle(hFile);
		}
		return;
	}

}


void AgAppLaunch(BYTE* Data)
{
	//Get information
	DWORD MagicNumber = 0, TotalSize = 0;
	WORD* PathR = NULL;
	size_t PathRSize = 0;
	DWORD offset = 0;


	MagicNumber = *(DWORD*)(Data);
	TotalSize = *(DWORD*)(Data + 4);


	TprintfC(Magenta, _T("MagicNumber:\t\t"));
	_tprintf(_T("%01X \n"), MagicNumber);

	TprintfC(Magenta, _T("Totale Size:\t\t"));
	_tprintf(_T("%04X - %d bytes  \n"), TotalSize, TotalSize);

	TprintfC(Magenta, _T("Entries: \n"));


	while (offset < TotalSize)
	{
		PathR = (WORD*)(Data + offset);
		PathRSize = _tcsclen(PathR);


		//we check the string is made with letters
		if (PathRSize > 7 && *(BYTE*)PathR > 65 && *(BYTE*)PathR < 91 && *(BYTE*)(PathR + 1) > 65 && *(BYTE*)(PathR + 1) < 91 && *(BYTE*)(PathR + 3) > 65 && *(BYTE*)(PathR + 3) < 91)
		{
			printf("\t\t\t%s\n", (CHAR*)PathR);
			offset += (DWORD)PathRSize + 1;
		}
		offset++;
	}
	return;
}



void GetInformation_Pf(LPTSTR lpFileName)
{
	HANDLE hFile = NULL;
	BYTE* Data = NULL;
	DWORD DataSize = 0;
	int ErrCode = 0;
	LPTSTR STR_Os = NULL;
	SYSTEMTIME* Timetab = (SYSTEMTIME*)calloc(8, sizeof(SYSTEMTIME));
	int j = 0;
	DWORD i = 0;
	WORD* PathR = NULL;
	size_t PathRSize = 0;
	BUFFER_PF* DataPf = NULL;

	TprintfC(Blue, _T("\n-------------------------\n"));
	TprintfC(Blue, _T("------ INFORMATION ------\n"));
	TprintfC(Blue, _T("-------------------------\n\n"));



	__try {

		//Check the header
		DataPf = ReadHeader(lpFileName);
		if (DataPf == NULL)
		{
			__leave;
		}
		//Open and read the file
		hFile = CreateFile(lpFileName, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-][-] Error: CreateFile(%s) %d.\n"), lpFileName, GetLastError());
			__leave;
		}

		DataSize = GetFileSize(hFile, NULL);
		if (DataSize == INVALID_FILE_SIZE) {
			TprintfC(Red, _T("[-][-] Error: GetFileSize %d.\n"), GetLastError());
			__leave;
		}

		Data = (BYTE*)calloc(DataSize, sizeof(BYTE));
		if (Data == NULL)
		{
			TprintfC(Red, _T("[-] Error: calloc() %d.\n"), GetLastError());
			__leave;
		}

		if (ReadFile(hFile, Data, DataSize, &DataSize, 0) == 0) {
			TprintfC(Red, _T("[-] Error: ReadFile(%s) %d.\n"), lpFileName, GetLastError());
			__leave;
		}
		TprintfC(Green, _T("[+] File viewed: %s.\n\n"), lpFileName);


		switch (DataPf->OS_id) {
		case '\x1e':
			STR_Os = _T("Windows 10");
			break;
		case '\x1a':
			STR_Os = _T("Windows 8");
			break;
		case '\x17':
			STR_Os = _T("Windows 7 - Xp - Vista");
			break;
		default:
			STR_Os = _T("Unkown os version");
		}

		for (j = 0; j < 8; j++)
		{
			FileTimeToSystemTime(&DataPf->Time[j], &Timetab[j]);
		}

		//Display information
		TprintfC(Magenta, _T("Format: \t\t "));
		_tprintf(_T("%02X - %s\n"), DataPf->OS_id, STR_Os);


		TprintfC(Magenta, _T("Compressed File Size:"));
		_tprintf(_T("\t% ld bytes\n"), DataPf->TotalSize);

		TprintfC(Magenta, _T("Program concerned:"));
		_tprintf(_T("\t %s \n"), DataPf->Name);

		TprintfC(Magenta, _T("Program hash:"));
		_tprintf(_T("\t\t %04X \n\n"), DataPf->HashValue);

		TprintfC(Magenta, _T("Count of executions:"));
		_tprintf(_T("\t %02d\n"), DataPf->CountRun);


		while (j > 0) {

			if (Timetab[j - 1].wYear != 1601) // if the date is defined
			{
				TprintfC(Magenta, _T("Last times executed :"));
				_tprintf(_T("\t %02d-%02d-%04d at %02d:%02d \n"),
					Timetab[j - 1].wDay, Timetab[j - 1].wMonth, Timetab[j - 1].wYear,
					Timetab[j - 1].wHour, Timetab[j - 1].wMinute);
			}


			j--;
		}
		TprintfC(Magenta, _T("\nCount of paths:"));
		_tprintf(_T("\t\t %02d \n"), DataPf->CountEntries);



		TprintfC(Magenta, _T("Entries:\t\t \n"));
		i = DataPf->OffsetEntry; //offset of the beginning of registered paths

		while (i < DataSize)
		{
			PathR = (WORD*)(Data + i);
			PathRSize = _tcsclen(PathR);

			if (PathRSize > 10 && _tcsstr((LPCTSTR)PathR, _T(".DLL")) == NULL)
			{
				if (_tcsstr((LPCTSTR)PathR, _T("\\")) != NULL)
				{
					_tprintf(_T("\t\t\t%s \n"), _tcsstr((LPTSTR)PathR, _T("\\")));
				}
			}
			i += (DWORD)PathRSize * 2 + 2;
		}


		__leave;
	}
	__finally {


		if (hFile) {
			CloseHandle(hFile);
		}
		return;
	}
}
