/*
Traffic Counter plugin for Miranda IM
Copyright 2007-2011 Mironych.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/* ======================================================================================
Здесь содержатся переменные и функции для работы со статистикой
Автор: Mironych
=======================================================================================*/

#include "stdafx.h"

OPTTREE_OPTION *pOptions; // Через этот указатель модуль opttree.c может добраться до списка опций.

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcPopupsTraffic(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		//
		CheckDlgButton(hwndDlg,IDC_NOTIFYSIZE,(unOptions.NotifyBySize) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg,IDC_ENOTIFYSIZE,Traffic_Notify_size_value,0);
		SendDlgItemMessage(hwndDlg,IDC_ENOTIFYSIZE,EM_LIMITTEXT,4,0);
		if (!unOptions.NotifyBySize)
			EnableWindow(GetDlgItem(hwndDlg,IDC_ENOTIFYSIZE),0);
		
		CheckDlgButton(hwndDlg,IDC_NOTIFYTIME,(unOptions.NotifyByTime) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg,IDC_ENOTIFYTIME,Traffic_Notify_time_value,0);
		SendDlgItemMessage(hwndDlg,IDC_ENOTIFYTIME,EM_LIMITTEXT,2,0);
		if (!unOptions.NotifyByTime)
			EnableWindow(GetDlgItem(hwndDlg,IDC_ENOTIFYTIME),0);

		//colors
		SendDlgItemMessage(hwndDlg,IDC_COLOR1,CPM_SETDEFAULTCOLOUR,0,RGB(200,255,200));
		SendDlgItemMessage(hwndDlg,IDC_COLOR1,CPM_SETCOLOUR,0,Traffic_PopupBkColor);
		SendDlgItemMessage(hwndDlg,IDC_COLOR2,CPM_SETDEFAULTCOLOUR,0,RGB(0,0,0));
		SendDlgItemMessage(hwndDlg,IDC_COLOR2,CPM_SETCOLOUR,0,Traffic_PopupFontColor);

		//timeout
		CheckDlgButton(hwndDlg,IDC_RADIO_FROMPOPUP,(Traffic_PopupTimeoutDefault != 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_RADIO_CUSTOM,(Traffic_PopupTimeoutDefault == 0) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg,IDC_POPUP_TIMEOUT),Traffic_PopupTimeoutDefault == 0);
		SetDlgItemInt(hwndDlg,IDC_POPUP_TIMEOUT,Traffic_PopupTimeoutValue,0);
		SendDlgItemMessage(hwndDlg,IDC_POPUP_TIMEOUT,EM_LIMITTEXT,2,0);
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_NOTIFYSIZE:
			EnableWindow(GetDlgItem(hwndDlg,IDC_ENOTIFYSIZE),IsDlgButtonChecked(hwndDlg,IDC_NOTIFYSIZE));
			break;

		case IDC_ENOTIFYSIZE:
			if (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) return 0;
			break;

		case IDC_NOTIFYTIME:
			EnableWindow(GetDlgItem(hwndDlg,IDC_ENOTIFYTIME),IsDlgButtonChecked(hwndDlg,IDC_NOTIFYTIME));
			break;

		case IDC_ENOTIFYTIME:
			if (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) return 0;
			break;

		case IDC_RESETCOLORS:
			SendDlgItemMessage(hwndDlg,IDC_COLOR1,CPM_SETDEFAULTCOLOUR,0,RGB(200,255,200));
			SendDlgItemMessage(hwndDlg,IDC_COLOR1,CPM_SETCOLOUR,0,RGB(200,255,200));
			SendDlgItemMessage(hwndDlg,IDC_COLOR2,CPM_SETDEFAULTCOLOUR,0,RGB(0,0,0));
			SendDlgItemMessage(hwndDlg,IDC_COLOR2,CPM_SETCOLOUR,0,RGB(0,0,0));
			break;

		case IDC_RADIO_FROMPOPUP:
		case IDC_RADIO_CUSTOM:
			EnableWindow(GetDlgItem(hwndDlg,IDC_POPUP_TIMEOUT),IsDlgButtonChecked(hwndDlg,IDC_RADIO_CUSTOM));
			break;

		case IDC_TEST:
			NotifyOnRecv();
			NotifyOnSend();
			return 0;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		return 0;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
				unOptions.NotifyBySize = IsDlgButtonChecked(hwndDlg,IDC_NOTIFYSIZE);
				Traffic_Notify_size_value = GetDlgItemInt(hwndDlg,IDC_ENOTIFYSIZE,nullptr,0);
				unOptions.NotifyByTime = IsDlgButtonChecked(hwndDlg,IDC_NOTIFYTIME);
				Traffic_Notify_time_value = GetDlgItemInt(hwndDlg,IDC_ENOTIFYTIME,nullptr,0);
				//
				Traffic_PopupBkColor = SendDlgItemMessage(hwndDlg,IDC_COLOR1,CPM_GETCOLOUR,0,0);
				Traffic_PopupFontColor = SendDlgItemMessage(hwndDlg,IDC_COLOR2,CPM_GETCOLOUR,0,0);
				//
				Traffic_PopupTimeoutDefault = IsDlgButtonChecked(hwndDlg,IDC_RADIO_FROMPOPUP);
				if (Traffic_PopupTimeoutDefault == 0) Traffic_PopupTimeoutValue = GetDlgItemInt(hwndDlg,IDC_POPUP_TIMEOUT,nullptr,0);
				//
				UpdateNotifyTimer();
				return TRUE;
			}
			break;
		}
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static OPTTREE_OPTION options[] =
{
	{0,	LPGENW("Display") L"/" LPGENW("Icon"), OPTTREE_CHECK,	OPTTREE_INVISIBLE, nullptr, "DrawProtoIcon"},
	{0,	LPGENW("Display") L"/" LPGENW("Account name"), OPTTREE_CHECK, OPTTREE_INVISIBLE, nullptr, "DrawProtoName"},
	{0,	LPGENW("Display") L"/" LPGENW("Current traffic"), OPTTREE_CHECK, OPTTREE_INVISIBLE, nullptr, "DrawCurrentTraffic"},
	{0,	LPGENW("Display") L"/" LPGENW("Total traffic"), OPTTREE_CHECK, OPTTREE_INVISIBLE, nullptr, "DrawTotalTraffic"},
	{0,	LPGENW("Display") L"/" LPGENW("Current online"), OPTTREE_CHECK, OPTTREE_INVISIBLE, nullptr, "DrawCurrentTime"},
	{0,	LPGENW("Display") L"/" LPGENW("Total online"),OPTTREE_CHECK, OPTTREE_INVISIBLE, nullptr, "DrawTotalTime"},
	{0,	LPGENW("General") L"/" LPGENW("Draw frame as skin element"), OPTTREE_CHECK, 1, nullptr, "DrawFrmAsSkin"},
	{0,	LPGENW("General") L"/" LPGENW("Show tooltip in traffic window"), OPTTREE_CHECK, 1, nullptr, "ShowTooltip"},
	{0,	LPGENW("General") L"/" LPGENW("\"Toggle traffic counter\" in main menu"), OPTTREE_CHECK, 1, nullptr, "ShowMainMenuItem"},
	// Резервируем место под активные и видимые протоколы
	// Максимум 16 позиций видимых и 16 активных
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	nullptr, OPTTREE_CHECK, OPTTREE_INVISIBLE,	nullptr, nullptr},
	{0,	LPGENW("Visible accounts")L"/" LPGENW("Summary traffic for visible accounts"), OPTTREE_CHECK, 1, nullptr, "ShowSummary"},
	{0,	LPGENW("Visible accounts")L"/" LPGENW("Overall traffic"), OPTTREE_CHECK, 1, nullptr, "ShowOverall"},
};

static INT_PTR CALLBACK DlgProcTCOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	uint16_t i, j;
	BOOL result;
	static uint8_t Initialized = 0;
	static uint16_t optionCount = 0;

	if (!Initialized) {
		pOptions = options;
		optionCount = _countof(options);
		// Если нет Variables, активируем галочки для старого метода рисования
		if (!bVariablesExists)
			for (i = 0; i < 8; i++)
				options[i].dwFlag = 1;

		// Флажки для видимости аккаунтов создаются в любом случае.
		for (i = j = 0; (j < NumberOfAccounts) && (i < optionCount) ; i++)
			if ((options[i].dwFlag & OPTTREE_INVISIBLE) && !options[i].szSettingName)
			{
				options[i].szSettingName = (char*)mir_alloc(1 + mir_strlen(ProtoList[j].name));
				mir_strcpy(options[i].szSettingName, ProtoList[j].name);
				size_t l = 20 + mir_wstrlen(ProtoList[j].tszAccountName);
				options[i].szOptionName = (wchar_t*)mir_alloc(sizeof(wchar_t) * l);
				mir_snwprintf(options[i].szOptionName, l, L"Visible accounts/%s", ProtoList[j].tszAccountName);
				options[i].dwFlag = (ProtoList[j++].Enabled ? 1 : OPTTREE_INVISIBLE) | OPTTREE_NOTRANSLATE;
			}
		Initialized = 1;
	}

	if (OptTree_ProcessMessage(hwndDlg, msg, wParam, lParam, &result, IDC_APPEARANCEOPTIONS, options, optionCount))
		return result;

	switch (msg) {	
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(15, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETPOS, 0, GetDlgItemInt(hwndDlg, IDC_EDIT_SPACE, nullptr, FALSE));

		//show/hide button
		SetDlgItemText(hwndDlg,IDC_BSHOWHIDE,(IsWindowVisible(TrafficHwnd) != 0)? TranslateT("Hide now") : TranslateT("Show now"));

		// Строки формата для счётчиков
		EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_COUNTER_FORMAT), bVariablesExists);
		SetDlgItemText(hwndDlg, IDC_EDIT_COUNTER_FORMAT, Traffic_CounterFormat);

		// Формат всплывающей подсказки
		EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_TOOLTIP_FORMAT), bTooltipExists);
		SetDlgItemText(hwndDlg, IDC_EDIT_TOOLTIP_FORMAT, Traffic_TooltipFormat);

		// Display traffic for current...
		SendDlgItemMessage(hwndDlg, IDC_COMBO_AUTO_CLEAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Day"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_AUTO_CLEAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Week"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_AUTO_CLEAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Month"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_AUTO_CLEAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Year"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_AUTO_CLEAR, CB_SETCURSEL, unOptions.PeriodForShow, 0);

		// Интервал между строками
		SetDlgItemInt(hwndDlg, IDC_EDIT_SPACE, Traffic_AdditionSpace, 0);
		SendDlgItemMessage(hwndDlg, IDC_EDIT_SPACE, EM_LIMITTEXT, 2, 0);

		// Appearance
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawProtoIcon, "DrawProtoIcon");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawProtoName, "DrawProtoName");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawCurrentTraffic, "DrawCurrentTraffic");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawTotalTraffic, "DrawTotalTraffic");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawCurrentTimeCounter, "DrawCurrentTime");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawTotalTimeCounter, "DrawTotalTime");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.DrawFrmAsSkin, "DrawFrmAsSkin");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.ShowSummary , "ShowSummary");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.ShowTooltip, "ShowTooltip");
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.ShowMainMenuItem, "ShowMainMenuItem");

		// Настройки видимости протоколов
		for (i = 0; i < NumberOfAccounts; i++) {
			char buffer[32];
			mir_strcpy(buffer, ProtoList[i].name);
			OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, ProtoList[i].Visible, buffer);
		}
		OptTree_SetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, unOptions.ShowOverall, "ShowOverall");

		EnableWindow(GetDlgItem(GetParent(hwndDlg),IDC_APPLY),FALSE);
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BSHOWHIDE:
			MenuCommand_TrafficShowHide(0, 0);
			SetDlgItemText(hwndDlg,IDC_BSHOWHIDE,IsWindowVisible(TrafficHwnd)? TranslateT("Hide now") : TranslateT("Show now"));
			return 0;

		case IDC_EDIT_SPACE:
		case IDC_EDIT_COUNTER_FORMAT:
		case IDC_EDIT_OVERALL_COUNTER_FORMAT:
		case IDC_EDIT_TOOLTIP_FORMAT:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_COMBO_AUTO_CLEAR:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		switch (lpnmhdr->code) {
		case PSN_APPLY:
			unOptions.PeriodForShow = (char)SendDlgItemMessage(hwndDlg,IDC_COMBO_AUTO_CLEAR,CB_GETCURSEL,0,0);

			// Интервал между строками
			Traffic_AdditionSpace = GetDlgItemInt(hwndDlg, IDC_EDIT_SPACE, nullptr, 0);
			// Настройки Appearance
			unOptions.DrawProtoIcon = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawProtoIcon");
			unOptions.DrawProtoName = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawProtoName");
			unOptions.DrawCurrentTraffic = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawCurrentTraffic");
			unOptions.DrawTotalTraffic = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawTotalTraffic");
			unOptions.DrawTotalTimeCounter = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawTotalTime");
			unOptions.DrawCurrentTimeCounter = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawCurrentTime");
			unOptions.DrawTotalTimeCounter = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawTotalTime");
			unOptions.DrawFrmAsSkin = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "DrawFrmAsSkin");
			unOptions.ShowSummary = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "ShowSummary");
			unOptions.ShowTooltip = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "ShowTooltip");
			unOptions.ShowMainMenuItem = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "ShowMainMenuItem");

			// Настройки видимости протоколов
			for (i = 0; i < NumberOfAccounts; i++)
			{
				char buffer[32];
				mir_strcpy(buffer, ProtoList[i].name);
				ProtoList[i].Visible = (uint8_t)OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, buffer);
			}
			unOptions.ShowOverall = OptTree_GetOptions(hwndDlg, IDC_APPEARANCEOPTIONS, options, optionCount, "ShowOverall");

			// Формат счётчиков
			GetDlgItemText(hwndDlg, IDC_EDIT_COUNTER_FORMAT, Traffic_CounterFormat, _countof(Traffic_CounterFormat));
			// Формат всплывающей подсказки
			GetDlgItemText(hwndDlg, IDC_EDIT_TOOLTIP_FORMAT, Traffic_TooltipFormat, _countof(Traffic_TooltipFormat));

			// Ключевой цвет
			UseKeyColor = db_get_b(0, "ModernSettings", "UseKeyColor", 1);
			KeyColor = db_get_dw(0, "ModernSettings", "KeyColor", 0);

			// Перерисовываем фрейм
			UpdateTrafficWindowSize();

			// Если отключается показ пункта главного меню, то удаляем его.
			if (!unOptions.ShowMainMenuItem && hTrafficMainMenuItem)
			{
				Menu_RemoveItem(hTrafficMainMenuItem);
				hTrafficMainMenuItem = nullptr;
			}
			// Если включается, то создаём.
			if (unOptions.ShowMainMenuItem && !hTrafficMainMenuItem)
				Traffic_AddMainMenuItem();

			SaveSettings(0);
			//
			return TRUE;
		}
	}
	return 0;
}

int TrafficCounterOptInitialise(WPARAM wParam, LPARAM)
{
	// Main options page
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 900000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRAFFIC);
	odp.szGroup.a = LPGEN("Services");
	odp.szTab.a = LPGEN("Options");
	odp.szTitle.a = LPGEN("Traffic counter");
	odp.pfnDlgProc = DlgProcTCOptions;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	// Statistics options page
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_STATS);
	odp.szTab.a = LPGEN("Statistics");
	odp.pfnDlgProc = DlgProcOptStatistics;
	g_plugin.addOptions(wParam, &odp);

	// Popups option page
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRAFFIC_POPUPS);
	odp.szGroup.a = LPGEN("Popups");
	odp.szTitle.a = LPGEN("Traffic counter");
	odp.pfnDlgProc = DlgProcPopupsTraffic;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
