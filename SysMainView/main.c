#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <tchar.h>
#include <wchar.h>
#include <winternl.h>
#include <uchar.h>
#include <shlobj_core.h>
#include <Shlobj.h>


#include "Utils.h"
#include "File.h"
#include "GetInfo.h"


#define _CRT_SECURE_DEPRECATE_MEMORY
#include <memory.h>



#define CHOICE_MAX 6


int menu();


//
// Display the menu, get the user's choice and returns it ; returns -1 if error
//
int menu() {

	int choice = 0;

	TprintfC(Blue, _T("\n-------------------------\n"));
	TprintfC(Blue, _T("-------- MENU -----------\n"));
	TprintfC(Blue, _T("-------------------------\n"));

	_tprintf(_T("1 - View Information.\n"));
	_tprintf(_T("2 - Decompress a pf/db file.\n"));
	_tprintf(_T("3 - Compress pf/db file.\n"));
	_tprintf(_T("4 - Modify a file.\n"));
	_tprintf(_T("5 - Hash a string.\n"));
	_tprintf(_T("--- Enter any other input to quit.\n\n"));
	_tprintf(_T("What would you like to do ?   "));
	scanf_s("%d", &choice);

	if (choice > 0 && choice < CHOICE_MAX) {

		_tprintf(_T("\n %d ; Let's do that!\n\n"), choice);
		return choice;

	}
	else {
		return -1;
	}


}



int _tmain(int argc, LPTSTR argv[]) {

	int choice = 0;
	LPTSTR name = NULL;

	printf("\n"

		"   _____           __  __       _               _   \n"
		"  / ____|         |  \\/  |     (_)             (_)   \n"
		" | (___  _   _ ___| \\  / | __ _ _ _ __   __   ___  _____      __ \n"
		" \\___  \\| | | / __| |\\/| |/ _` | | '_ \\  \\ \\ / / |/ _ \\ \\ /\\ / /\n"
		"  ____) | |_| \\__ \\ |  | | (_| | | | | |  \\ V /| |  __/\\ V  V / \n"
		" |_____/ \\__, |___/_|  |_|\\__,_|_|_| |_|   \\_/ |_|\\___| \\_/\\_/  \n"
		"         __/ |                                                 \n"
		"        |___/                                                  \n"

		"\n");

	while (choice != -1) {
		name = NULL;
		choice = menu();
		if (choice > 0 && choice < 5)
		{
			_tprintf(_T("Please choose a file."));

			name = SelectTargetFileByExplorer();
			if (name == NULL)
			{
				TprintfC(Red, _T("Error getting the file.\n"));
				continue;
			}
		}

		switch (choice) {

		case 1:
			switch (IsDbFile(name))
			{
			case 0:
				GetInformation_Pf(name);
				break;
			case 1:
				GetInformation_Db(name);
				break;
			case -1:
				break;
			}
			break;
		case 2:
			DecompressFile(name);
			break;
		case 3:
			CompressFile(name);
			break;
		case 4:
			ModifyFile(name);
			break;
		case 5:
			HashCmd();
			break;
		default:
			break;
		}
	}

	return 0;
}

