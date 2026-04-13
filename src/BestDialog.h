#pragma once

#include <wx/dialog.h>

namespace wxmine {

struct Preferences;

// Loads ID_DLG_BEST from XRC and populates it with the three best-time rows
// from Preferences. The Reset Scores button (ID_BTN_RESET) clears them.
class BestDialog : public wxDialog {
public:
    BestDialog(wxWindow* parent, Preferences& prefs);

private:
    void Refresh();
    void OnReset(wxCommandEvent& evt);

    Preferences& m_prefs;
};

} // namespace wxmine
