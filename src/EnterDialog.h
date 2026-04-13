#pragma once

#include <wx/dialog.h>
#include <wx/string.h>

namespace wxmine {

struct Preferences;

// Pops up after a player beats their personal best for one of the three
// fixed difficulty levels. Pre-fills the edit field with the previous
// holder's name; on OK, writes the new name back into Preferences.
class EnterDialog : public wxDialog {
public:
    EnterDialog(wxWindow* parent, Preferences& prefs, int level);

private:
    void OnOk(wxCommandEvent& evt);

    Preferences& m_prefs;
    int          m_level;
};

} // namespace wxmine
