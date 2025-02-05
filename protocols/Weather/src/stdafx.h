/*
Weather Protocol plugin for Miranda NG
Copyright (C) 2012-25 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This file contains the includes, weather constants/declarations,
   the structs, and the primitives for some of the functions.
*/

#pragma once

#include <share.h>
#include <time.h>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_acc.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_clc.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_findadd.h>
#include <m_fontservice.h>
#include <m_history.h>
#include <m_icolib.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_skin_eng.h>
#include <m_userinfo.h>
#include <m_xstatus.h>

#include <m_tipper.h>
#include <m_toptoolbar.h>

#include "resource.h"
#include "version.h"
#include "proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS

// name
#define MODULENAME         "Weather"
#define DEFCURRENTWEATHER  "WeatherCondition"
#define WEATHERCONDITION   "Current"

// weather conditions
enum EWeatherCondition
{
	SUNNY,
	NA,
	PCLOUDY,
	CLOUDY,
	RAIN,
	RSHOWER,
	FOG,
	SNOW,
	SSHOWER,
	LIGHT,
	MAX_COND
};

// limits
#define MAX_TEXT_SIZE   4096
#define MAX_DATA_LEN    1024

// db info mangement mode
#define WDBM_REMOVE			1
#define WDBM_DETAILDISPLAY	2

// more info list column width
#define LIST_COLUMN		 150

// others
#define NODATA			TranslateT("N/A")
#define UM_SETCONTACT	40000

// weather update error codes
#define INVALID_ID_FORMAT  10
#define INVALID_SVC        11
#define INVALID_ID         12
#define SVC_NOT_FOUND      20
#define NETLIB_ERROR       30
#define DATA_EMPTY         40
#define DOC_NOT_FOUND      42
#define DOC_TOO_SHORT      43
#define UNKNOWN_ERROR      99

#define SM_WEATHERALERT		16
#define WM_UPDATEDATA      (WM_USER + 2687)

// defaults constants
#define VAR_LIST_OPT TranslateT("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temp\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set\n----------\n\\n\tnew line")

/////////////////////////////////////////////////////////////////////////////////////////
// DATA FORMAT STRUCT

#define WID_NORMAL	0
#define WID_SET		1
#define WID_BREAK	2

struct WIDATAITEM
{
	wchar_t *Name;
	wchar_t *Start;
	wchar_t *End;
	wchar_t *Unit;
	char    *Url;
	wchar_t *Break;
	int      Type;
};

struct WITEMLIST
{
	WIDATAITEM Item;
	struct WITEMLIST *Next;
};

typedef struct WITEMLIST WIDATAITEMLIST;

struct WIIDSEARCH
{
	BOOL Available;
	char *SearchURL;
	wchar_t *NotFoundStr;
	WIDATAITEM Name;
};

struct WINAMESEARCHTYPE
{
	BOOL Available;
	wchar_t *First;
	WIDATAITEM Name;
	WIDATAITEM ID;
};

struct WINAMESEARCH
{
	char *SearchURL;
	wchar_t *NotFoundStr;
	wchar_t *SingleStr;
	WINAMESEARCHTYPE Single;
	WINAMESEARCHTYPE Multiple;
};

struct STRLIST
{
	wchar_t *Item;
	struct STRLIST *Next;
};

typedef struct STRLIST WICONDITEM;

struct WICONDLIST
{
	WICONDITEM *Head;
	WICONDITEM *Tail;
};

struct WIDATA
{
	wchar_t *FileName;
	wchar_t *ShortFileName;
	BOOL Enabled;

	// header
	wchar_t *DisplayName;
	wchar_t *InternalName;
	wchar_t *Description;
	wchar_t *Author;
	wchar_t *Version;
	int InternalVer;
	size_t MemUsed;

	// default
	char  *DefaultURL;
	wchar_t *DefaultMap;
	char  *UpdateURL;
	char  *UpdateURL2;
	char  *UpdateURL3;
	char  *UpdateURL4;
	char  *Cookie;
	char  *UserAgent;

	// items
	int UpdateDataCount;
	WIDATAITEMLIST *UpdateData;
	WIDATAITEMLIST *UpdateDataTail;
	WIIDSEARCH IDSearch;
	WINAMESEARCH NameSearch;
	WICONDLIST CondList[MAX_COND];
};

/////////////////////////////////////////////////////////////////////////////////////////
// DATA LIST (LINKED LIST)

struct DATALIST
{
	WIDATA Data;
	struct DATALIST *next;
};

typedef struct DATALIST WIDATALIST;

/////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES

extern WIDATALIST *WIHead, *WITail;

extern HWND hPopupWindow, hWndSetup;

extern MWindowList hDataWindowList, hWindowList;

extern HANDLE hTBButton;

extern HGENMENU hMwinMenu;

extern bool g_bIsUtf;

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_conv.c

void ClearStatusIcons();

uint16_t GetIcon(const wchar_t* cond, WIDATA *Data);
void CaseConv(wchar_t *str);
void TrimString(char *str);
void TrimString(wchar_t *str);
void ConvertBackslashes(char *str);
char *GetSearchStr(char *dis);

wchar_t *GetDisplay(WEATHERINFO *w, const wchar_t *dis, wchar_t* str);

void GetSvc(wchar_t *pszID);
void GetID(wchar_t *pszID);

wchar_t *GetError(int code);

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_data.c

int DBGetData(MCONTACT hContact, char *setting, DBVARIANT *dbv);

void wSetData(char *&Data, const char *Value);
void wSetData(wchar_t *&Data, const char *Value);
void wSetData(wchar_t *&Data, const wchar_t *Value);
void wfree(char *&Data);
void wfree(wchar_t *&Data);

void DBDataManage(MCONTACT hContact, uint16_t Mode, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_ini.c

WIDATA* GetWIData(wchar_t *pszServ);

bool IsContainedInCondList(const wchar_t *pszStr, WICONDLIST *List);

void DestroyWIList();
bool LoadWIData(bool dial);

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_info.c

void GetINIInfo(wchar_t *pszSvc);
wchar_t* GetINIVersionNum(int iVersion);
const wchar_t *GetDefaultText(int c);

void MoreVarList();

/////////////////////////////////////////////////////////////////////////////////////////
// function from multiwin module

void UpdateMwinData(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// UI Classes

class WeatherMyDetailsDlg : public CUserInfoPageDlg
{
	CCtrlButton btnReload;

public:
	WeatherMyDetailsDlg();

	bool OnInitDialog() override;

	void onClick_Reload(CCtrlButton *);
};
