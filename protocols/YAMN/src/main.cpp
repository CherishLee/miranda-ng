/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 * YAMN homepage: http://www.majvan.host.sk/Projekty/YAMN
 *
 * initializes all variables for further work
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

 //--------------------------------------------------------------------------------------------------

wchar_t ProfileName[MAX_PATH];
wchar_t UserDirectory[MAX_PATH];

wchar_t	szMirandaDir[MAX_PATH];
wchar_t	szProfileDir[MAX_PATH];

BOOL UninstallPlugins;

HANDLE hAccountFolder;

HINSTANCE *hDllPlugins;
static int iDllPlugins = 0;

YAMN_VARIABLES YAMNVar;

CMPlugin	g_plugin;

HANDLE hNewMailHook;
HANDLE NoWriterEV;
HANDLE hTTButton;

UINT SecTimer;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

static void GetProfileDirectory(wchar_t *szPath, int cbPath)
//This is copied from Miranda's sources. In 0.2.1.0 it is needed, in newer vesions of Miranda use MS_DB_GETPROFILEPATH service
{
	wchar_t tszOldPath[MAX_PATH];
	Profile_GetPathW(_countof(tszOldPath), tszOldPath);
	mir_wstrcat(tszOldPath, L"\\*.book");

	VARSW ptszNewPath(L"%miranda_userdata%");

	SHFILEOPSTRUCT file_op = {
		nullptr,
		FO_MOVE,
		tszOldPath,
		ptszNewPath,
		FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT,
		false,
		nullptr,
		L""};
	SHFileOperation(&file_op);

	wcsncpy(szPath, ptszNewPath, cbPath);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {B047A7E5-027A-4CFC-8B18-EDA8345D2790}
	{0xb047a7e5, 0x27a, 0x4cfc, {0x8b, 0x18, 0xed, 0xa8, 0x34, 0x5d, 0x27, 0x90}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(YAMN_DBMODULE, pluginInfoEx)
{
	RegisterProtocol(PROTOTYPE_VIRTUAL);
	SetUniqueId("Id");
}

/////////////////////////////////////////////////////////////////////////////////////////
// The callback function

BOOL CALLBACK EnumSystemCodePagesProc(LPTSTR cpStr)
{
	// Convert code page string to number
	UINT cp = _wtoi(cpStr);
	if (!IsValidCodePage(cp))
		return TRUE;

	// Get Code Page name
	CPINFOEX info;
	if (GetCPInfoEx(cp, 0, &info)) {
		for (int i = 1; i < CPLENALL; i++) if (CodePageNamesAll[i].CP == cp) {
			CodePageNamesAll[i].isValid = TRUE;
			CPLENSUPP++;
			break;
		}
	}
	return TRUE;
}

int SystemModulesLoaded(WPARAM, LPARAM)
{
	//Insert "Check mail (YAMN)" item to Miranda's menu
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0xa01ff3d9, 0x53cb, 0x4406, 0x85, 0xd9, 0xf1, 0x90, 0x3a, 0x94, 0xed, 0xf4);
	mi.position = 0xb0000000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_CHECKMAIL);
	mi.name.a = LPGEN("Check &mail (All Account)");
	mi.pszService = MS_YAMN_FORCECHECK;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xfe22191f, 0x40c8, 0x479f, 0x93, 0x5d, 0xa5, 0x17, 0x1f, 0x57, 0x2f, 0xcb);
	mi.name.a = LPGEN("Check &mail (This Account)");
	mi.pszService = MS_YAMN_CLISTCONTEXT;
	Menu_AddContactMenuItem(&mi, YAMN_DBMODULE);

	SET_UID(mi, 0x147c7800, 0x12d0, 0x4209, 0xab, 0xcc, 0xfa, 0x64, 0xc6, 0xb0, 0xa6, 0xeb);
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_LAUNCHAPP);
	mi.name.a = LPGEN("Launch application");
	mi.pszService = MS_YAMN_CLISTCONTEXTAPP;
	Menu_AddContactMenuItem(&mi, YAMN_DBMODULE);

	if (hAccountFolder = FoldersRegisterCustomPathW(LPGEN("YAMN"), LPGEN("YAMN Account Folder"), UserDirectory))
		FoldersGetCustomPathW(hAccountFolder, UserDirectory, MAX_PATH, UserDirectory);

	RegisterPOP3Plugin(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{LPGEN("Check mail"), "YAMN_Check", IDI_CHECKMAIL},
	{LPGEN("Launch application"), "YAMN_Launch", IDI_LAUNCHAPP},
	{LPGEN("New Mail"), "YAMN_NewMail", IDI_NEWMAIL},
	{LPGEN("Connect Fail"), "YAMN_ConnectFail", IDI_BADCONNECT},
};

void LoadIcons()
{
	g_plugin.registerIcon("YAMN", iconList);
}

static void LoadPlugins()
{
	wchar_t szSearchPath[MAX_PATH];
	mir_snwprintf(szSearchPath, L"%s\\Plugins\\YAMN\\*.dll", szMirandaDir);

	hDllPlugins = nullptr;

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szSearchPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			//rewritten from Miranda sources... Needed because Win32 API has a bug in FindFirstFile, search is done for *.dlllllll... too
			wchar_t *dot = wcsrchr(fd.cFileName, '.');
			if (dot == nullptr)
				continue;

			// we have a dot
			int len = (int)mir_wstrlen(fd.cFileName); // find the length of the string
			wchar_t *end = fd.cFileName + len; // get a pointer to the NULL
			int safe = (end - dot) - 1;	// figure out how many chars after the dot are "safe", not including NULL

			if ((safe != 3) || (mir_wstrcmpi(dot + 1, L"dll") != 0)) //not bound, however the "dll" string should mean only 3 chars are compared
				continue;

			wchar_t szPluginPath[MAX_PATH];
			mir_snwprintf(szPluginPath, L"%s\\Plugins\\YAMN\\%s", szMirandaDir, fd.cFileName);
			HINSTANCE hDll = LoadLibrary(szPluginPath);
			if (hDll == nullptr)
				continue;

			LOADFILTERFCN LoadFilter = (LOADFILTERFCN)GetProcAddress(hDll, "LoadFilter");
			if (nullptr == LoadFilter) {
				FreeLibrary(hDll);
				hDll = nullptr;
				continue;
			}

			if (!LoadFilter(GetFcnPtrSvc)) {
				FreeLibrary(hDll);
				hDll = nullptr;
			}

			if (hDll != nullptr) {
				hDllPlugins = (HINSTANCE *)realloc(hDllPlugins, (iDllPlugins + 1) * sizeof(HINSTANCE));
				hDllPlugins[iDllPlugins++] = hDll;
			}
		} while (FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}

int CMPlugin::Load()
{
	//	we get the Miranda Root Path
	PathToAbsoluteW(L".", szMirandaDir);

	// retrieve the current profile name
	Profile_GetNameW(_countof(ProfileName), ProfileName);
	wchar_t *fc = wcsrchr(ProfileName, '.');
	if (fc != nullptr) *fc = 0;

	//	we get the user path where our yamn-account.book.ini is stored from mirandaboot.ini file
	GetProfileDirectory(UserDirectory, _countof(UserDirectory));

	// Enumerate all the code pages available for the System Locale
	EnumSystemCodePages(EnumSystemCodePagesProc, CP_INSTALLED);
	CodePageNamesSupp = new _tcptable[CPLENSUPP];
	for (int i = 0, k = 0; i < CPLENALL; i++)
		if (CodePageNamesAll[i].isValid) {
			CodePageNamesSupp[k] = CodePageNamesAll[i];
			k++;
		}

	if (nullptr == (NoWriterEV = CreateEvent(nullptr, TRUE, TRUE, nullptr)))
		return 1;
	if (nullptr == (WriteToFileEV = CreateEvent(nullptr, FALSE, FALSE, nullptr)))
		return 1;
	if (nullptr == (ExitEV = CreateEvent(nullptr, TRUE, FALSE, nullptr)))
		return 1;

	PosX = g_plugin.getDword(YAMN_DBPOSX, 0);
	PosY = g_plugin.getDword(YAMN_DBPOSY, 0);
	SizeX = g_plugin.getDword(YAMN_DBSIZEX, 800);
	SizeY = g_plugin.getDword(YAMN_DBSIZEY, 200);

	HeadPosX = g_plugin.getDword(YAMN_DBMSGPOSX, 0);
	HeadPosY = g_plugin.getDword(YAMN_DBMSGPOSY, 0);
	HeadSizeX = g_plugin.getDword(YAMN_DBMSGSIZEX, 690);
	HeadSizeY = g_plugin.getDword(YAMN_DBMSGSIZEY, 300);
	HeadSplitPos = g_plugin.getWord(YAMN_DBMSGPOSSPLIT, 250);

	optDateTime = g_plugin.getByte(YAMN_DBTIMEOPTIONS, optDateTime);

	// Create new window queues for broadcast messages
	YAMNVar.MessageWnds = WindowList_Create();
	YAMNVar.NewMailAccountWnd = WindowList_Create();
	YAMNVar.Shutdown = FALSE;

	hCurSplitNS = LoadCursor(nullptr, IDC_SIZENS);
	hCurSplitWE = LoadCursor(nullptr, IDC_SIZEWE);

	#ifdef _DEBUG
	InitDebug();
	#endif

	CreateServiceFunctions();

	g_plugin.addSound(YAMN_NEWMAILSOUND, L"YAMN", YAMN_NEWMAILSNDDESC);
	g_plugin.addSound(YAMN_CONNECTFAILSOUND, L"YAMN", YAMN_CONNECTFAILSNDDESC);

	HookEvents();

	LoadIcons();
	LoadPlugins();

	HOTKEYDESC hkd = {};
	hkd.pszName = "YAMN_hotkey";
	hkd.pszService = MS_YAMN_FORCECHECK;
	hkd.szSection.a = YAMN_DBMODULE;
	hkd.szDescription.a = LPGEN("Check mail");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F11);
	g_plugin.addHotkey(&hkd);

	//Create thread that will be executed every second
	if (!(SecTimer = SetTimer(nullptr, 0, 1000, TimerProc)))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void UnloadPlugins()
{
	if (hDllPlugins == nullptr)
		return;

	for (int i = iDllPlugins - 1; i >= 0; i--) {
		if (FreeLibrary(hDllPlugins[i])) {
			hDllPlugins[i] = nullptr;				//for safety
			iDllPlugins--;
		}
	}
	free((void *)hDllPlugins);
	hDllPlugins = nullptr;
}

int CMPlugin::Unload()
{
	#ifdef _DEBUG
	UnInitDebug();
	#endif

	WindowList_Destroy(YAMNVar.MessageWnds);
	WindowList_Destroy(YAMNVar.NewMailAccountWnd);

	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurSplitWE);

	CloseHandle(NoWriterEV);
	CloseHandle(WriteToFileEV);
	CloseHandle(ExitEV);

	UnloadPlugins();

	delete[] CodePageNamesSupp;
	return 0;
}
