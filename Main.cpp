#undef UNICODE
#include <windows.h>
#include "resource.h"
#include <iostream>
#include <stdio.h> 

using namespace std;

HANDLE handle = 0;


BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
// PID - ���������� ������������� �������� 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainWndProc);
	return 0;
}

BOOL RunNotepad(HWND hWnd) { // ������ �������
	char processName[50]; // ��� ������ "Notepad.exe"
	int processNameLength = sizeof(processName);
	if (!GetDlgItemText(hWnd, IDC_COMMANDLINE, processName, processNameLength)) // ������ ��� �� �������� � ������ � ��������� �����
		return 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si)); // �������� ����� ������������� ��������
	si.cb = sizeof(si);

	if (!CreateProcess(
		NULL,
		processName, // ��������� ������
		NULL,
		NULL,
		FALSE,
		0,   // ������ ��������
		NULL,
		NULL,
		&si, // ���������� ������������� 
		&pi) // ���������� ��������
		)return 0;
	
		if(handle != 0) CloseHandle(handle);
		CloseHandle(pi.hThread);
		handle = pi.hProcess;
		return 1; // return true
}



VOID ShowStartInfo(HWND hWnd) { // ����� �������� ����� ������� ����
	
	FILETIME creation_time, exit_time, kernel_time, user_time; // ����� �������� ��������, ����� ���������� ��������, ����� ������ �������� � ������ ���� ?, ����� ������ �������� � ������ ������������.
	if(!GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time)) return;// � ������� GetProcessTimes ������� ���������� � ������������� ���������� �������
	FILETIME creation_time_1;
	FileTimeToLocalFileTime(&creation_time, &creation_time_1); // ����������� ����� ����� � ��������� ����� �����... ���������� �� ��������� ������� ��������� � creation_time_1
	SYSTEMTIME systemTime; // ������ ��������� ����� ��� �������� �������������
	if (!FileTimeToSystemTime(&creation_time_1, &systemTime))return; // ������������ �������� ����� � ��������� ����� 

	const int display_time = 50; // ������������ ������ �������
	char time_string[display_time];
	sprintf_s(time_string, display_time, "%02u : %02u  %02u/%02u/%04u", systemTime.wHour, systemTime.wMinute, systemTime.wDay, systemTime.wMonth, systemTime.wYear);
	
	SetDlgItemText(hWnd, IDC_STARTTIME, time_string);
	SetDlgItemText(hWnd, IDC_FINISHTIME, "");
	SetDlgItemText(hWnd, IDC_STATE, "Running");

	GetProcessId(handle); // ��������� ������������� �������� ���������� ��������.
	SetDlgItemInt(hWnd, IDC_PID, GetProcessId(handle), TRUE); // ������� ��� ������������� � ���� IDC_PID
	
	
}


VOID ShowExitInfo(HWND hWnd) { // ����� ������� ��������� ����� ���������, ���� ����� ��� � ������ � ShowStartInfo ������ ������� � IDC_FINISH ����� ����� ���������� ��������

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
	ofn.lStructSize = sizeof(ofn); // ���������� ������ � ������ !!! 
	ofn.hwndOwner = Wnd;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0" // �������������� ������, ������� ��������� ������
						"All Files(*.*)\0 * .*\0"; //  ���������� ������ �������
	ofn.lpstrFile = FileName; // ��������� �� �����, ������� �������� ��� �����
	ofn.nMaxFile = MAX_PATH; // ������ ������
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "exe"; 

	return GetOpenFileName(&ofn); // ��������� ���������� ���� � ������� ��������� ����������� ����, ��� �����...
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

