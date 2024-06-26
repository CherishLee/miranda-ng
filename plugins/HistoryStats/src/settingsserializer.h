#if !defined(HISTORYSTATS_GUARD_SETTINGSSERIALIZER_H)
#define HISTORYSTATS_GUARD_SETTINGSSERIALIZER_H

#include "stdafx.h"
#include "_consts.h"

#include "settings.h"

class SettingsSerializer
	: public Settings
	, private pattern::NotCopyable<SettingsSerializer>
{
private:
	uint32_t m_VersionInDB;
	MirandaSettings m_DB;

public:
	explicit SettingsSerializer(const char* module);

	void readFromDB();
	void writeToDB();

	bool isDBUpdateNeeded();

	int getLastPage();
	void setLastPage(int nPage);

	bool getShowColumnInfo();
	void setShowColumnInfo(bool bShow);

	bool getShowSupportInfo();
	void setShowSupportInfo(bool bShow);

	ext::string getLastStatisticsFile();
	void setLastStatisticsFile(const wchar_t* szFileName);
	bool canShowStatistics();
	void showStatistics();
};

#endif // HISTORYSTATS_GUARD_SETTINGSSERIALIZER_H