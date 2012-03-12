#include "searchbar.h"

#include "utils/log.h"

namespace Gejengel
{

SearchBar::SearchBar()
: m_Label("Search:")
, m_CloseImage(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_SMALL_TOOLBAR)
{
	m_SearchEntry.set_icon_from_stock(Gtk::Stock::FIND, Gtk::ENTRY_ICON_PRIMARY);
	m_SearchEntry.set_icon_from_stock(Gtk::Stock::CLOSE, Gtk::ENTRY_ICON_SECONDARY);
	m_SearchEntry.set_icon_activatable(Gtk::ENTRY_ICON_SECONDARY);
	
	m_SearchEntry.signal_changed().connect(sigc::mem_fun(*this, &SearchBar::onSearchChanged));
	m_SearchEntry.signal_icon_press().connect(sigc::mem_fun(*this, &SearchBar::onSearchIconPress));
	m_SearchEntry.signal_key_press_event().connect(sigc::mem_fun(*this, &SearchBar::onKeyPress), false);
	
	pack_start(m_Label, Gtk::PACK_SHRINK);
	pack_start(m_SearchEntry, Gtk::PACK_EXPAND_WIDGET);
	set_spacing(5);
}

void SearchBar::onSearchChanged()
{
    signalSearchChanged.emit(m_SearchEntry.get_text());
}

void SearchBar::setFocus()
{
	m_SearchEntry.grab_focus();
}

bool SearchBar::onKeyPress(GdkEventKey* pEvent)
{
    switch (pEvent->keyval)
    {
	case GDK_Escape:
	{
		cancelSearch();
		return true;
	}
    default:
        return false;
    }
}

void SearchBar::onSearchIconPress(Gtk::EntryIconPosition iconPos, const GdkEventButton*)
{
    if (iconPos == Gtk::ENTRY_ICON_SECONDARY)
    {
        cancelSearch();
    }
}

void SearchBar::cancelSearch()
{
	if (!m_SearchEntry.get_text().empty())
	{
		m_SearchEntry.set_text("");
	}

	signalClose.emit();
}

}
