#include "stdafx.h"

HWND filterAddDlg = nullptr;

static unsigned __stdcall filterQueue(void *dummy);
static INT_PTR CALLBACK ConnectionFilterEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HANDLE startFilterThread()
{
	return (HANDLE)mir_forkthreadex(filterQueue, nullptr, (unsigned int*)&FilterOptionsThreadId);
}

static unsigned __stdcall filterQueue(void *)
{
	BOOL bRet;
	MSG msg;
	//while(1)
	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (msg.message == WM_ADD_FILTER)
		{
			CONNECTION *conn = (CONNECTION *)msg.lParam;
			filterAddDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER_DIALOG), nullptr, ConnectionFilterEditProc, (LPARAM)conn);
			ShowWindow(filterAddDlg, SW_SHOW);

		}
		if (nullptr == filterAddDlg || !IsDialogMessage(filterAddDlg, &msg)) { /* Wine fix. */
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	hFilterOptionsThread = nullptr;
	return TRUE;
}

static INT_PTR CALLBACK ConnectionFilterEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		CONNECTION *conn = (CONNECTION*)lParam;
		TranslateDialogDefault(hWnd);

		SetDlgItemText(hWnd, ID_TEXT_NAME, conn->PName);
		SetDlgItemText(hWnd, ID_TXT_LOCAL_IP, conn->strIntIp);
		SetDlgItemText(hWnd, ID_TXT_REMOTE_IP, conn->strExtIp);
		SetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT, conn->intIntPort, FALSE);
		SetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT, conn->intExtPort, FALSE);
		SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Always show popup"));
		SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Never show popup"));
		SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_SETCURSEL, 0, 0);
		mir_free(conn);
		return TRUE;
	}
	case WM_ACTIVATE:
		if (0 == wParam)             // becoming inactive
			filterAddDlg = nullptr;
		else                         // becoming active
			filterAddDlg = hWnd;
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_OK:
		{
			wchar_t tmpPort[6];
			if (bOptionsOpen)
			{
				MessageBox(hWnd, TranslateT("First close options window"), L"ConnectionNotify", MB_OK | MB_ICONSTOP);
				break;
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(hExceptionsMutex, 100))
			{
				if (connCurrentEdit == nullptr)
				{
					connCurrentEdit = (CONNECTION*)mir_alloc(sizeof(CONNECTION));
					connCurrentEdit->next = connExceptions;
					connExceptions = connCurrentEdit;
				}
				GetDlgItemText(hWnd, ID_TXT_LOCAL_PORT, tmpPort, _countof(tmpPort));
				if (tmpPort[0] == '*')
					connCurrentEdit->intIntPort = -1;
				else
					connCurrentEdit->intIntPort = GetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT, nullptr, FALSE);
				GetDlgItemText(hWnd, ID_TXT_REMOTE_PORT, tmpPort, _countof(tmpPort));
				if (tmpPort[0] == '*')
					connCurrentEdit->intExtPort = -1;
				else
					connCurrentEdit->intExtPort = GetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT, nullptr, FALSE);

				GetDlgItemText(hWnd, ID_TXT_LOCAL_IP, connCurrentEdit->strIntIp, _countof(connCurrentEdit->strIntIp));
				GetDlgItemText(hWnd, ID_TXT_REMOTE_IP, connCurrentEdit->strExtIp, _countof(connCurrentEdit->strExtIp));
				GetDlgItemText(hWnd, ID_TEXT_NAME, connCurrentEdit->PName, _countof(connCurrentEdit->PName));

				connCurrentEdit->Pid = !(BOOL)SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_GETCURSEL, 0, 0);
				connCurrentEdit = nullptr;
				saveSettingsConnections(connExceptions);
				ReleaseMutex(hExceptionsMutex);
			}
			//EndDialog(hWnd,IDOK);
			DestroyWindow(hWnd);
			return TRUE;
		}
		case ID_CANCEL:
			connCurrentEdit = nullptr;
			DestroyWindow(hWnd);
			//EndDialog(hWnd,IDCANCEL);
			return TRUE;
		}
		return FALSE;

		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
	case WM_DESTROY:
		filterAddDlg = nullptr;
		connCurrentEdit = nullptr;
		//DestroyWindow(hWnd);
		//PostQuitMessage(0);
		break;
	}
	return FALSE;
}

BOOL checkFilter(CONNECTION *head, CONNECTION *conn)
{
	for (CONNECTION *cur = head; cur != nullptr; cur = cur->next)
		if (wildcmpw(conn->PName, cur->PName) && wildcmpw(conn->strIntIp, cur->strIntIp) && wildcmpw(conn->strExtIp, cur->strExtIp)
			&& (cur->intIntPort == -1 || cur->intIntPort == conn->intIntPort) && (cur->intExtPort == -1 || cur->intExtPort == conn->intExtPort))
			return cur->Pid;

	return g_plugin.iDefaultAction;
}

