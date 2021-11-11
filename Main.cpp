#undef UNICODE
#include <windows.h>
#include "resource.h"
#include <iostream>
#include <stdio.h> 

using namespace std;

HANDLE handle = 0;


BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
// PID - уникальный идентификатор процесса 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainWndProc);
	return 0;
}

BOOL RunNotepad(HWND hWnd) { // создаём процесс
	char processName[50]; // идёт вместо "Notepad.exe"
	int processNameLength = sizeof(processName);
	if (!GetDlgItemText(hWnd, IDC_COMMANDLINE, processName, processNameLength)) // читает что мы написали в строку и открывает прогу
		return 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si)); // избегаем любых нежелательных эффектов
	si.cb = sizeof(si);

	if (!CreateProcess(
		NULL,
		processName, // командная строка
		NULL,
		NULL,
		FALSE,
		0,   // флажки создание
		NULL,
		NULL,
		&si, // информация предустановки 
		&pi) // информация процесса
		)return 0;
	
		if(handle != 0) CloseHandle(handle);
		CloseHandle(pi.hThread);
		handle = pi.hProcess;
		return 1; // return true
}



VOID ShowStartInfo(HWND hWnd) { // будет работать когда создаем файл
	
	FILETIME creation_time, exit_time, kernel_time, user_time; // время создание процесса, время завершение процесса, время работы процесса в режиме ядра ?, время работы процесса в режиме пользователя.
	if(!GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time)) return;// с помощью GetProcessTimes получаю информацию о распределение интервалов времени
	FILETIME creation_time_1;
	FileTimeToLocalFileTime(&creation_time, &creation_time_1); // переобразую время файла в локальное время файла... информацию об локальном времени записиваю в creation_time_1
	SYSTEMTIME systemTime; // создаём системное время для будущего использование
	if (!FileTimeToSystemTime(&creation_time_1, &systemTime))return; // переобразуем файловое время в системное время 

	const int display_time = 50; // максимальный размер времени
	char time_string[display_time];
	sprintf_s(time_string, display_time, "%02u : %02u  %02u/%02u/%04u", systemTime.wHour, systemTime.wMinute, systemTime.wDay, systemTime.wMonth, systemTime.wYear);
	
	SetDlgItemText(hWnd, IDC_STARTTIME, time_string);
	SetDlgItemText(hWnd, IDC_FINISHTIME, "");
	SetDlgItemText(hWnd, IDC_STATE, "Running");

	GetProcessId(handle); // Извлекает идентификатор процесса указанного процесса.
	SetDlgItemInt(hWnd, IDC_PID, GetProcessId(handle), TRUE); // Выводим это идентификатор в поле IDC_PID
	
	
}


VOID ShowExitInfo(HWND hWnd) { // здесь вызываю конеччное время программы, тоже самое что и делали в ShowStartInfo только выводим в IDC_FINISH время после завершение процесса

	FILETIME creation_time, exit_time, kernel_time, user_time;
	if (!GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time)) return;
	FILETIME exit_time_1;
	FileTimeToLocalFileTime(&exit_time, &exit_time_1);
	SYSTEMTIME systemTime;
	if (!FileTimeToSystemTime(&exit_time_1, &systemTime))return;

	const int display_time = 50;
	char time_string[display_time];
	sprintf_s(time_string, display_time, "%02u : %02u  %02u/%02u/%04u", systemTime.wHour, systemTime.wMinute, systemTime.wDay, systemTime.wMonth, systemTime.wYear);

	SetDlgItemText(hWnd, IDC_FINISHTIME, time_string);
	SetDlgItemText(hWnd, IDC_STATE, "Finished");
	
}
bool BrowseFileName(HWND Wnd, char* FileName) {
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn); // определяет строку в БАЙТАХ !!! 
	ofn.hwndOwner = Wnd;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0" // информационная строка, которая описывает фильтр
						"All Files(*.*)\0 * .*\0"; //  определяет модель фильтра
	ofn.lpstrFile = FileName; // Указатель на буфер, который содержит имя файла
	ofn.nMaxFile = MAX_PATH; // Размер буфера
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "exe"; 

	return GetOpenFileName(&ofn); // возращаем диалоговое окно в котором находится открываемый диск, имя файла...
}


BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_INITDIALOG:
		SetTimer(hWnd, NULL, 250, NULL);		
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			DestroyWindow(hWnd);
			return TRUE;
		case IDC_BROWSE: {
			char filename[MAX_PATH] = "notepad.exe";
			if (BrowseFileName(hWnd, filename)) SetDlgItemText(hWnd, IDC_COMMANDLINE, filename);
			return TRUE;
		} return FALSE;
		case IDC_START:
			if (handle != 0) return TRUE;
			if (!RunNotepad(hWnd))return TRUE;
			ShowStartInfo(hWnd);
			return TRUE;
		case IDC_TERMINATE:
			if (handle != 0) 
				TerminateProcess(handle, 1);
			return FALSE;
		}
		return TRUE;

	case WM_TIMER:
		if (handle == NULL) return FALSE;
		DWORD exit_process;
		GetExitCodeProcess(handle, &exit_process);
		if (exit_process == STILL_ACTIVE) return TRUE;//
		ShowExitInfo(hWnd);
		return TRUE;

	case WM_DESTROY:
		if (handle != 0) CloseHandle(handle);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;

}

