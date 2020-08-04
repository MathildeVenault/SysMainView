#include "File.h"

//
//  Takes a buffer and adds the value at the place determined thanks to the parameters
//	returns 0 or the error code of the problem if there is one
//
DWORD Addat(DWORD Offset, BYTE** Input, DWORD* InputSize, DWORD* ToAdd)
{
	BYTE* output = NULL;
	DWORD i = 0;
	DWORD SizeOutput = 4; //because dword as an entry
	DWORD InputSizecp = *InputSize;


	while (InputSizecp > 0)
	{
		SizeOutput++;
		InputSizecp--;
	}

	output = (BYTE*)calloc(SizeOutput, sizeof(BYTE));
	if (output == NULL)
	{
		TprintfC(Red, _T("[-] Error: calloc() %d.\n"), GetLastError());
		return GetLastError();
	}

	*(DWORD*)output = *(DWORD*)ToAdd;

	if (memcpy_s(output + 4, SizeOutput, *Input, *InputSize))
	{
		TprintfC(Red, _T("[-] Error: memcpy() %d.\n"), GetLastError());
		return GetLastError();
	}

	*Input = output;
	*InputSize = SizeOutput;
	return 0;
}

//
// Display the content of a file on hexa values from the offset given in parameter 
//
void PrintBuffer(BYTE* Input, DWORD InputSize, DWORD NbLines, DWORD From)
{
	DWORD i = 0, j = 0;

	TprintfC(Yellow, _T("           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F \n"));

	for (i = 0; i < NbLines; i++)
	{
		j = 0;
		TprintfC(Yellow, _T(" %08X"), From + i * 16);
		for (j = 0; j < 16; j++)
		{
			if ((From + i * 16 + j) < InputSize)
			{
				_tprintf(_T(" %02X"), Input[From + i * 16 + j]);
			}
			else
			{
				_tprintf(_T("  "));
			}
		}
		_tprintf(_T("\n"));
	}
}

//
// Determines if the name of the file on the argument is a .db of .pf and returns: 
//   1 for .db
//   0 for .pf
//   -1 if error
//
DWORD IsDbFile(LPTSTR FileToD)
{
	size_t size_path = 0;

	size_path = _tcslen(FileToD);

	if (FileToD[size_path - 1] == 'f')
	{
		return 0;
	}
	else if (FileToD[size_path - 1] == 'b')
	{
		return 1;
	}
	else
	{
		TprintfC(Red, _T("[-] Wrong file type.\n"));
		return -1;
	}
}

//
// Determines if the buffer in arg corresponds to a compressed file and return TRUE if it does	
//
BOOL IsFileCompressed(BYTE* Data)
{
	if ((DWORD)*Data == 0x4D)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//
// Take the buffer corresponding to the data file and the offset of the hour to change all the dates & times recorded
// return 0 if no error
//
DWORD ModifyAllHours(BYTE* DataDecomp)
{
	FILETIME Filet_new;
	SYSTEMTIME Syst_new;
	DWORD DateOffset = 184;
	int i = 0;

	//for all the dates on the file
	for (i = 0; i < 8; i++)
	{
		//get the new value to replace
		_tprintf(_T("\nEnter the new year:"));
		scanf_s("%hd", &Syst_new.wYear);
		if (Syst_new.wYear < 2010)
		{
			TprintfC(Red, _T("[-] Invalid year.\n"));
			return 1;
		}
		_tprintf(_T("Enter the new month:"));
		scanf_s("%hd", &Syst_new.wMonth);
		if (Syst_new.wMonth < 1 || Syst_new.wMonth > 12)
		{
			TprintfC(Red, _T("[-] Invalid month.\n"));
			return 1;
		}
		_tprintf(_T("Enter the new day:"));
		scanf_s("%hd", &Syst_new.wDay);
		if (Syst_new.wDay < 0 || Syst_new.wDay > 31)
		{
			TprintfC(Red, _T("[-] Invalid day.\n"));
			return 1;
		}
		_tprintf(_T("Enter the new hour (pm format):"));
		scanf_s("%hd", &Syst_new.wHour);
		if (Syst_new.wHour < 0 || Syst_new.wHour > 23)
		{
			TprintfC(Red, _T("[-] Invalid hour.\n"));
			return 1;
		}
		_tprintf(_T("Enter the new minutes:"));
		scanf_s("%hd", &Syst_new.wMinute);
		if (Syst_new.wMinute < 0 || Syst_new.wMinute > 60)
		{
			TprintfC(Red, _T("[-] Invalid minutes.\n"));
			return 1;
		}
		_tprintf(_T("Enter the new seconds:"));
		scanf_s("%hd", &Syst_new.wSecond);
		if (Syst_new.wSecond < 0 || Syst_new.wSecond > 60)
		{
			TprintfC(Red, _T("[-] Invalid seconds.\n"));
			return 1;
		}
		Syst_new.wMilliseconds = 0;

		//convert
		if (SystemTimeToFileTime(&Syst_new, &Filet_new) == 0)
		{
			TprintfC(Red, _T("[-] Error SystemTimeToFileTime() %d.\n"), GetLastError());
			return 1;
		}
		// let the user knows
		TprintfC(Green, _T("\n [+] Your  date %d: %02d-%02d-%04d at %02d:%02d has been added.\n\n"), i,
			Syst_new.wDay, Syst_new.wMonth, Syst_new.wYear,
			Syst_new.wHour, Syst_new.wMinute);

		*(DWORD*)(DataDecomp + DateOffset) = Filet_new.dwLowDateTime;
		*(DWORD*)(DataDecomp + DateOffset + 4) = Filet_new.dwHighDateTime;
		//we go to the next date
		DateOffset -= 8;

	}
	return 0;
}

//
// Take the buffer corresponding to the data file and the offset of the hour to change with making sure the change respects time cohenrency
//
DWORD ModifyHours(BYTE* DataDecomp, DWORD offset)
{
	//if the value to change is time
	FILETIME Filet_TimeToChange;
	SYSTEMTIME Syst_TimeToChange;
	FILETIME Filet_LastTime;
	SYSTEMTIME Syst_LastTime;
	FILETIME Filet_NextTime;
	SYSTEMTIME Syst_NextTime;
	FILETIME Filet_new;
	SYSTEMTIME Syst_new;
	DWORD onyldate = 0;
	__try {
		//get the date to change
		Filet_TimeToChange.dwHighDateTime = *(DWORD*)(DataDecomp + offset + 4);
		Filet_TimeToChange.dwLowDateTime = *(DWORD*)(DataDecomp + offset);
		if (FileTimeToSystemTime(&Filet_TimeToChange, &Syst_TimeToChange) == 0)
		{
			TprintfC(Red, _T("[-] Error FileTimeToSystemTime() 1: %d.\n"), GetLastError());
			__leave;
		}
		_tprintf(_T("\nYou're about to change a date: %02d-%02d-%04d at %02d:%02d \n\n"),
			Syst_TimeToChange.wDay, Syst_TimeToChange.wMonth, Syst_TimeToChange.wYear,
			Syst_TimeToChange.wHour, Syst_TimeToChange.wMinute);

		if (*(DWORD*)(DataDecomp + offset + 12) == 0) // if there is no other date
		{
			onyldate = 1;
		}

		//if the date is not the oldest
		if (offset != 184 && *(DWORD*)(DataDecomp + offset + 12) != 0)
		{
			//get the date just before 
			Filet_LastTime.dwHighDateTime = *(DWORD*)(DataDecomp + offset + 12);
			Filet_LastTime.dwLowDateTime = *(DWORD*)(DataDecomp + offset + 8);
			FileTimeToSystemTime(&Filet_LastTime, &Syst_LastTime);

			_tprintf(_T("Please make sure the date is later than:  %02d-%02d-%04d at %02d:%02d \n\n"),
				Syst_LastTime.wDay, Syst_LastTime.wMonth, Syst_LastTime.wYear,
				Syst_LastTime.wHour, Syst_LastTime.wMinute);
		}
		if (offset != 128)
		{
			//get the date just after 
			Filet_NextTime.dwHighDateTime = *(DWORD*)(DataDecomp + offset - 4);
			Filet_NextTime.dwLowDateTime = *(DWORD*)(DataDecomp + offset - 8);
			if (FileTimeToSystemTime(&Filet_NextTime, &Syst_NextTime) == 0)
			{
				TprintfC(Red, _T("[-] Error FileTimeToSystemTime() 2: %d.\n"), GetLastError());
				__leave;
			}

			_tprintf(_T("Please make sure the date is earlier than: %02d-%02d-%04d at %02d:%02d \n\n"),
				Syst_NextTime.wDay, Syst_NextTime.wMonth, Syst_NextTime.wYear,
				Syst_NextTime.wHour, Syst_NextTime.wMinute);

		}
		//get the new value to replace
		_tprintf(_T("\n| Enter the new year:"));
		scanf_s("%hd", &Syst_new.wYear);
		if (Syst_new.wYear < 1600)
		{
			TprintfC(Red, _T("[-] Invalid year.\n"));
			__leave;
		}
		_tprintf(_T("| Enter the new month:"));
		scanf_s("%hd", &Syst_new.wMonth);
		if (Syst_new.wMonth < 1 || Syst_new.wMonth > 12)
		{
			TprintfC(Red, _T("[-] Invalid month.\n"));
			__leave;
		}
		_tprintf(_T("| Enter the new day:"));
		scanf_s("%hd", &Syst_new.wDay);
		if (Syst_new.wDay < 1 || Syst_new.wDay > 31)
		{
			TprintfC(Red, _T("[-] Invalid day.\n"));
			__leave;
		}
		_tprintf(_T("| Enter the new hour (pm format):"));
		scanf_s("%hd", &Syst_new.wHour);
		if (Syst_new.wHour < 1 || Syst_new.wHour > 23)
		{
			TprintfC(Red, _T("[-] Invalid hour.\n"));
			__leave;
		}
		_tprintf(_T("| Enter the new minutes:"));
		scanf_s("%hd", &Syst_new.wMinute);
		if (Syst_new.wMinute < 1 || Syst_new.wMinute > 60)
		{
			TprintfC(Red, _T("[-] Invalid minutes.\n"));
			__leave;
		}
		_tprintf(_T("| Enter the new seconds:"));
		scanf_s("%hd", &Syst_new.wSecond);
		if (Syst_new.wSecond < 1 || Syst_new.wSecond > 60)
		{
			TprintfC(Red, _T("[-] Invalid seconds.\n"));
			__leave;
		}
		Syst_new.wMilliseconds = 0;

		//convert
		if (SystemTimeToFileTime(&Syst_new, &Filet_new) == 0)
		{
			TprintfC(Red, _T("[-] Error SystemTimeToFileTime() %d.\n"), GetLastError());
			__leave;
		}

		*(DWORD*)(DataDecomp + offset + 4) = Filet_new.dwHighDateTime;
		*(DWORD*)(DataDecomp + offset) = Filet_new.dwLowDateTime;

		// let the user knows
		TprintfC(Green, _T("\n [+] Your new date: %02d-%02d-%04d at %02d:%02d has been added.\n\n"),
			Syst_new.wDay, Syst_new.wMonth, Syst_new.wYear,
			Syst_new.wHour, Syst_new.wMinute);
	}
	__finally {

		return GetLastError();
	}
}

//
//  Takes an existing decompressed file and offer to the user to modify bytes and saves
//
void ModifyFile(LPTSTR FileToD)
{

	HANDLE hFile = NULL;
	BYTE* DataDecomp = NULL;
	DWORD  DataDecomp_Size = 0;
	int ErrCode = 0;
	DWORD nbBytesWritten = 0;
	size_t size_path = 0;
	DWORD ToAdd = 0, offset = 0;
	WORD value = 0;
	TCHAR ChangeAllDates[2] = { 'n' };
	TCHAR save[2] = { 'y' };
	TCHAR compress[2] = { 'y' };

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
	_tprintf(_T("\n-------------------------\n"));
	_tprintf(_T("------- EDIT DATA -------\n"));
	_tprintf(_T("--------------------------\n"));

	__try {

		//Checking it's a db file 
		if (IsDbFile(FileToD) == -1)
		{
			__leave;
		}

		//Opening the existing file
		hFile = CreateFile(FileToD, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile(%s) %d.\n"), FileToD, GetLastError());
			__leave;
		}

		DataDecomp_Size = GetFileSize(hFile, NULL);
		if (DataDecomp_Size == INVALID_FILE_SIZE) {
			TprintfC(Red, _T("[-] Error: GetFileSize %d.\n"), GetLastError());
			__leave;
		}

		DataDecomp = (BYTE*)calloc(DataDecomp_Size, sizeof(BYTE));
		if (DataDecomp == NULL)
		{
			TprintfC(Red, _T("[-] Error: calloc() %d.\n"), GetLastError());
			__leave;
		}

		if (ReadFile(hFile, DataDecomp, DataDecomp_Size, &DataDecomp_Size, 0) == 0) {
			TprintfC(Red, _T("[-] Error: ReadFile(%s) %d.\n"), FileToD, GetLastError());
			__leave;
		}
		CloseHandle(hFile);
		hFile = NULL;

		TprintfC(Green, _T("\n[+] File to edit: %s.\n\n"), FileToD);

		//initial display
		PrintBuffer(DataDecomp, DataDecomp_Size, 20, 0);
		DWORD oldoffset = 0;
		while (offset != -1)
		{
			rewind(stdin);
			_tprintf(_T("\nEnter the offset where you want to change (in hexadecimal), or -1 if you have finished. "));

			scanf_s(("%08X"), &offset);
			//if the user wants to stop
			if (offset == -1)
			{
				break;
			}
			else if (offset > DataDecomp_Size || (signed int)offset < -1)
			{
				TprintfC(Red, _T("[-] Invalid offset. Please try again.\n"));
				offset = 0;
			}
			//for all the valid values
			else
			{
				//if the data the user wants to change is not displayed yet
				if (abs((signed int)offset - (signed int)oldoffset) > 20)
				{
					if ((signed int)offset - 5 > 0)
					{
						PrintBuffer(DataDecomp, DataDecomp_Size, 20, offset - 5 * 16);
					}
					else
					{
						PrintBuffer(DataDecomp, DataDecomp_Size, 20, 0);
					}
				}
				oldoffset = offset;
				//if the user wants to change the date of a pf
				if ((IsDbFile(FileToD) == 0) && (offset == 128 || offset == 136 || offset == 144 || offset == 152 || offset == 160 || offset == 168 || offset == 176 || offset == 184))
				{
					if (offset == 184)
					{
						rewind(stdin);
						_tprintf(_T("\nWould you like to change all the 8 dates & times ? (Y/n) "));
						_tscanf_s(_T("%wS"), &ChangeAllDates, (unsigned int)_countof(ChangeAllDates));
					}
					if (ChangeAllDates[0] != _T('n') && ChangeAllDates[0] != _T('N') && ChangeAllDates[0] != _T('Y') && ChangeAllDates[0] != _T('y')) {
						TprintfC(Red, _T("[-] Wrong choice, back to menu.\n"));
						__leave;
					}

					if (ChangeAllDates[0] == _T('n') || ChangeAllDates[0] == _T('N'))
					{
						//1 time change
						if (ModifyHours(DataDecomp, offset) != 0)
						{
							__leave;
						}
					}
					if (ChangeAllDates[0] == _T('Y') || ChangeAllDates[0] == _T('y'))
					{
						if (ModifyAllHours(DataDecomp) != 0)
						{
							__leave;
						}
					}
					break;
				}
			}

			_tprintf(_T("\nEnter the value to replace %02X at offset %08X:"), *(DWORD*)(DataDecomp + offset), offset);
			rewind(stdin);
			scanf_s(("%02X"), &value);

			*(WORD*)(DataDecomp + offset) = value;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
			_tprintf(_T("\n-------------------------\n"));
			_tprintf(_T("----- CURRENT STATE -----\n"));
			_tprintf(_T("--------------------------\n"));


			if ((signed int)offset - 5 > 0)
			{
				PrintBuffer(DataDecomp, DataDecomp_Size, 20, offset - 5 * 16);
			}
			else
			{
				PrintBuffer(DataDecomp, DataDecomp_Size, 20, 0);
			}


		}
		//Save the data if needed
		rewind(stdin);
		_tprintf(_T("\nWould you like to save it ? (Y/n) "));
		_tscanf_s(_T("%wS"), &save, (unsigned int)_countof(save));

		if (save[0] != _T('n') && save[0] != _T('N') && save[0] != _T('Y') && save[0] != _T('y')) {
			TprintfC(Red, _T("[-] Wrong choice, back to menu.\n"));
			__leave;
		}

		if (save[0] == _T('n') || save[0] == _T('N'))
		{
			__leave;

		}
		if (save[0] == _T('Y') || save[0] == _T('y'))
		{
			hFile = CreateFile(FileToD, FILE_WRITE_DATA, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) {
				TprintfC(Red, _T("[-] Error: CreateFile(%s) %d.\n"), FileToD, GetLastError());
				__leave;
			}
			if (!WriteFile(hFile, DataDecomp, DataDecomp_Size, &nbBytesWritten, NULL))
			{
				TprintfC(Red, _T("[-] Error: WriteFile %d.\n"), GetLastError());
				__leave;
			}
			TprintfC(Green, _T("\n[+] File saved  : %s.\n\n"), FileToD);


		}

		//Compress the new file if needed 		
		rewind(stdin);
		_tprintf(_T("Would you like to compress the file ? (Y/n) "));
		_tscanf_s(_T("%wS"), &compress, (unsigned int)_countof(compress));

		if (compress[0] != _T('n') && compress[0] != _T('N') && compress[0] != _T('Y') && compress[0] != _T('y')) {
			TprintfC(Red, _T("[-] Wrong choice, back to menu.\n"));
			__leave;
		}
		else if (compress[0] == _T('n') || compress[0] == _T('N'))
		{
			__leave;
		}
		else if (compress[0] == _T('Y') || compress[0] == _T('y'))
		{

			CloseHandle(hFile);
			hFile = NULL;
			CompressFile(FileToD);
		}
	}

	__finally
	{
		if (hFile != NULL)
		{
			CloseHandle(hFile);
		}
	}
	return;
}

//
// Takes a decompressed file (.pf or .db), compresses adding information required. 
//
void CompressFile(LPTSTR FileToD) {

	LPTSTR lpFileName = NULL;
	LPCTSTR extension = _T("compressed.pf");
	HANDLE hFile = NULL;
	BYTE* DataComp, * DataDecomp = NULL;
	DWORD DataComp_Size = 0, DataDecomp_Size = 0;
	int ErrCode = 0;
	DWORD nbBytesWritten = 0;
	size_t size_path = 0, size_ext = 0;
	int i = 0, j = 0, IsPf = 1;
	TCHAR MoreInformation = 'y'; // by default information will be displayed
	DWORD ToAdd = 0, Checksum = 0, IsDb = -1;

	TprintfC(Blue, _T("\n-------------------------\n"));
	TprintfC(Blue, _T("------ COMPRESSION ------"));
	TprintfC(Blue, _T("\n-------------------------\n"));

	__try {

		//Setting if pf or db file 
		IsDb = IsDbFile(FileToD);
		if (IsDb == -1)
		{
			__leave;
		}
		if (IsDb == 1)
		{
			extension = _T("compressed.db");
		}

		// Calculating the name of output file compressed
		size_path = _tcslen(FileToD);
		size_ext = _tcslen(extension);

		lpFileName = (LPTSTR)calloc(size_path + size_ext, sizeof(TCHAR));
		if (lpFileName == NULL)
		{
			TprintfC(Red, _T("[-] Error: calloc() %d.\n"), GetLastError());
			__leave;
		}

		if (wmemcpy_s(lpFileName, size_path + size_ext, (LPCTSTR)FileToD, size_path - 3))
		{
			TprintfC(Red, _T("[-] Error: memcpy_s() %d.\n"), GetLastError());
			__leave;
		}


		if (wcscat_s(lpFileName, size_path + size_ext, extension))
		{
			TprintfC(Red, _T("[-] Error: scat_s() %d.\n"), GetLastError());
			__leave;

		}

		TprintfC(Green, _T("\n[+] File to compress : %s.\n\n"), FileToD);


		//Opening & reading the existing file
		hFile = CreateFile(FileToD, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile(%s) n°1 %d.\n"), FileToD, GetLastError());
			__leave;
		}

		DataDecomp_Size = GetFileSize(hFile, NULL);
		if (DataDecomp_Size == INVALID_FILE_SIZE) {
			TprintfC(Red, _T("[-] Error: GetFileSize() %d.\n"), GetLastError());
			__leave;
		}

		DataDecomp = (BYTE*)calloc(DataDecomp_Size, sizeof(BYTE));
		DataComp = (BYTE*)calloc(DataDecomp_Size, sizeof(BYTE));

		if (ReadFile(hFile, DataDecomp, DataDecomp_Size, &DataDecomp_Size, 0) == 0) {
			TprintfC(Red, _T("[-] Error: ReadFile(%s) %d.\n"), FileToD, GetLastError());
			__leave;
		}

		//check if the data is already compressed
		if (IsFileCompressed(DataDecomp))
		{
			TprintfC(Red, _T("[-] File already compressed.\n"));
			__leave;
		}


		//Compressing & writing the compressed data
		DataComp = BufferCompress(DataDecomp, DataDecomp_Size, DataComp, DataDecomp_Size, &DataComp_Size);
		if (DataDecomp == NULL)
		{
			TprintfC(Red, _T("[-] Error: BufferCompress %d.\n"), GetLastError());
			__leave;

		}

		//We add what's the classic 8 bytes header of the compressed format
		ToAdd = 0;

		if (IsDb)
		{
			if (Addat(0, &DataComp, &DataComp_Size, &ToAdd))
			{
				TprintfC(Red, _T("[-] Error: Addat 0 ; %d.\n"), GetLastError());
				__leave;
			}

			ToAdd = 2219655501; //magic number of db
		}
		else
		{
			ToAdd = 72171853; //magic number of pf
		}
		if (Addat(0, &DataComp, &DataComp_Size, &DataDecomp_Size))
		{
			TprintfC(Red, _T("[-] Error: Addat 1 ; %d.\n"), GetLastError());
			__leave;
		}


		if (Addat(0, &DataComp, &DataComp_Size, &ToAdd))
		{
			TprintfC(Red, _T("[-] Error: Addat 2 ; %d.\n"), GetLastError());
			__leave;
		}

		if (IsDb)
		{
			Checksum = CalculChecksum(DataComp, DataComp_Size);
			if (Checksum == 0)
			{
				TprintfC(Red, _T("[-] Error: Claculating the checksum; %d.\n"), GetLastError());
				__leave;
			}
			*(DWORD*)(DataComp + 8) = Checksum;
		}


		hFile = NULL;

		//Writing the compressed buffer
		hFile = CreateFile(lpFileName, FILE_WRITE_DATA, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile(%s) %d.\n"), lpFileName, GetLastError());
			__leave;
		}

		ErrCode = WriteFile(hFile, DataComp, DataComp_Size, &nbBytesWritten, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: WriteFile %d.\n"), GetLastError());
			__leave;
		}

		TprintfC(Green, _T("\n[+] File compressed succesfully : %s.\n\n"), lpFileName);

		_tprintf(_T("Extract:\n"));


		//Display how it looks like
		PrintBuffer(DataComp, DataComp_Size, 10, 0);

		CloseHandle(hFile);
		hFile = NULL;

	}
	__finally
	{
		if (hFile) {
			CloseHandle(hFile);
		}
		return;
	}
}

//
// Takes a compressed file (.pf or .db), decompresses it and offers to view the information/
//
void DecompressFile(LPTSTR FileToD) {

	LPTSTR lpFileName = NULL;
	LPCTSTR extension = _T("_result.pf");
	HANDLE hFile = NULL;
	BYTE* DataComp, * DataDecomp;
	DWORD DataComp_Size = 0, DataDecomp_Size = 0;
	int ErrCode = 0;
	DWORD nbBytesWritten = 0;
	size_t size_path = 0, size_ext = 0;
	int i = 0, j = 0, IsDb = 1;
	TCHAR MoreInformation[2] = { 'y' };
	TCHAR temp = { '\0' };

	TprintfC(Blue, _T("\n-------------------------\n"));
	TprintfC(Blue, _T("----- DECOMPRESSION -----"));
	TprintfC(Blue, _T("\n-------------------------\n"));

	__try {

		//Setting if pf or db file 
		IsDb = IsDbFile(FileToD);
		if (IsDb == -1)
		{
			__leave;
		}

		if (IsDb == 1)
		{
			extension = _T("_result.db");
		}

		// Calculating the name of output file 
		size_path = _tcslen(FileToD);
		size_ext = _tcslen(extension);

		lpFileName = (LPTSTR)calloc(size_path + size_ext, sizeof(TCHAR));
		if (lpFileName == NULL)
		{
			TprintfC(Red, _T("[-] Error: calloc() %d.\n"), GetLastError());
			__leave;
		}

		if (wmemcpy_s(lpFileName, size_path + size_ext, (LPCTSTR)FileToD, size_path - 3))
		{
			TprintfC(Red, _T("[-] Error: wmemcpy() %d.\n"), GetLastError());
			__leave;
		}

		if (wcscat_s(lpFileName, size_path + size_ext, extension))
		{
			TprintfC(Red, _T("[-] Error: wscat() %d.\n"), GetLastError());
			__leave;
		}
		TprintfC(Green, _T("\n[+] File to decompress : %s.\n\n"), FileToD);

		//Opening th existing file
		hFile = CreateFile(FileToD, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile(%s) %d.\n"), FileToD, GetLastError());

			__leave;
		}

		DataComp_Size = GetFileSize(hFile, NULL);
		if (DataComp_Size == INVALID_FILE_SIZE) {
			TprintfC(Red, _T("[-] Error: GetFileSize() %d.\n"), GetLastError());
			__leave;
		}

		DataComp = (BYTE*)calloc(DataComp_Size, sizeof(BYTE));

		if (ReadFile(hFile, DataComp, DataComp_Size, &DataComp_Size, 0) == 0) {
			TprintfC(Red, _T("[-] Error: ReadFile(%s) %d.\n"), FileToD, GetLastError());
			__leave;
		}

		//check if the file is already decompressed
		if (IsFileCompressed(DataComp) == FALSE)
		{
			TprintfC(Red, _T("[-] File  already decompressed.\n"));
			__leave;
		}

		//Decompresing the data from the original file
		DataDecomp_Size = (*(DWORD*)(DataComp + 4));
		DataDecomp = (BYTE*)calloc(DataDecomp_Size, sizeof(BYTE));

		if (IsDb == 1)
		{
			DataComp += 12;
			DataComp_Size -= 12;
		}
		else if (IsDb == 0)
		{
			DataComp += 8;
			DataComp_Size -= 8;
		}

		DataDecomp = BufferDecompress(DataComp, DataComp_Size, DataDecomp, DataDecomp_Size);
		if (DataDecomp == NULL)
		{
			TprintfC(Red, _T("[-] Error: BufferDecompress() %d.\n"), GetLastError());
			__leave;
		}

		//Writing into the new file
		hFile = NULL;
		hFile = CreateFile(lpFileName, FILE_WRITE_DATA, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: CreateFile() %d.\n"), GetLastError());
			__leave;
		}

		ErrCode = WriteFile(hFile, DataDecomp, DataDecomp_Size, &nbBytesWritten, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			TprintfC(Red, _T("[-] Error: WriteFile() %d.\n"), GetLastError());
			__leave;
		}

		TprintfC(Green, _T("[+] File decompressed and built: %s.\n\n"), lpFileName);

		CloseHandle(hFile);
		hFile = NULL;

		//End of decompressing part 


		//Getting the user's wish to see more
		_tprintf(_T("Do you want to parse the file ? (Y/n)"));
		rewind(stdin);
		_tscanf_s(_T("%wS"), &MoreInformation, (unsigned int)_countof(MoreInformation));

		if (_tcslen(&MoreInformation[0]) == 0) {
			TprintfC(Red, _T("[-] Your choice has not been understood.\n"));
			__leave;
		}

		if (MoreInformation[0] != _T('n') && MoreInformation[0] != _T('N') && MoreInformation[0] != _T('Y') && MoreInformation[0] != _T('y')) {
			TprintfC(Red, _T("[-] Wrong choice, back to menu.\n"));
			__leave;
		}

		if (MoreInformation[0] == _T('n') || MoreInformation[0] == _T('N'))
		{
			__leave;

		}
		if (MoreInformation[0] == _T('Y') || MoreInformation[0] == _T('y'))
		{
			if (IsDb == 0)
			{
				GetInformation_Pf(lpFileName);
			}
			else if (IsDb == 1)
			{
				GetInformation_Db(lpFileName);
			}
		}
		CloseHandle(hFile);
		hFile = NULL;

	}
	__finally
	{
		if (hFile) {
			CloseHandle(hFile);
		}
		return;
	}

}

