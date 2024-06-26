#if !defined(HISTORYSTATS_GUARD_COLUMN_NICK_H)
#define HISTORYSTATS_GUARD_COLUMN_NICK_H

#include "column.h"

/*
 * ColNick
 */

class ColNick
	: public Column
{
private:
	bool m_bDetail;
	bool m_bContactCount;

	OptionsCtrl::Check m_hDetail;
	OptionsCtrl::Check m_hContactCount;

public:
	explicit ColNick();

protected:
	virtual const wchar_t* impl_getUID() const { return con::ColNick; }
	virtual const wchar_t* impl_getTitle() const { return LPGENW("Nick"); }
	virtual const wchar_t* impl_getDescription() const { return LPGENW("Column holding the contact's nick and first/last message time if selected."); }
	virtual void impl_copyConfig(const Column* pSource);
	virtual int impl_getFeatures() const { return cfHasConfig; }
	virtual void impl_configRead(const SettingsTree& settings);
	virtual void impl_configWrite(SettingsTree& settings) const;
	virtual void impl_configToUI(OptionsCtrl& Opt, OptionsCtrl::Item hGroup);
	virtual void impl_configFromUI(OptionsCtrl& Opt);
	virtual int impl_configGetRestrictions(ext::string*) const { return crHTMLFull; }
	virtual void impl_outputRenderHeader(ext::ostream& tos, int row, int rowSpan) const;
	virtual void impl_outputRenderRow(ext::ostream& tos, const CContact& contact, DisplayType display);
};

#endif // HISTORYSTATS_GUARD_COLUMN_NICK_H