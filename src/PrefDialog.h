#pragma once

#include <wx/dialog.h>

namespace wxmine {

struct Preferences;

// Loads ID_DLG_PREF from XRC. After ShowModal() returns wxID_OK, reads the
// validated Height/Width/Mines values back into the supplied prefs struct.
class PrefDialog : public wxDialog {
public:
    PrefDialog(wxWindow* parent, Preferences& prefs);

private:
    void OnOk(wxCommandEvent& evt);

    Preferences& m_prefs;
};

} // namespace wxmine
