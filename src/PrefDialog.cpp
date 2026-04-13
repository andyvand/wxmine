#include "PrefDialog.h"

#include <wx/textctrl.h>
#include <wx/valnum.h>
#include <wx/xrc/xmlres.h>

#include "GameConstants.h"
#include "Preferences.h"

namespace wxmine {

PrefDialog::PrefDialog(wxWindow* parent, Preferences& prefs)
    : m_prefs(prefs)
{
    wxXmlResource::Get()->LoadDialog(this, parent, "ID_DLG_PREF");

    XRCCTRL(*this, "ID_EDIT_HEIGHT", wxTextCtrl)->SetValue(wxString::Format("%d", prefs.Height));
    XRCCTRL(*this, "ID_EDIT_WIDTH",  wxTextCtrl)->SetValue(wxString::Format("%d", prefs.Width));
    XRCCTRL(*this, "ID_EDIT_MINES",  wxTextCtrl)->SetValue(wxString::Format("%d", prefs.Mines));

    Bind(wxEVT_BUTTON, &PrefDialog::OnOk, this, wxID_OK);
}

void PrefDialog::OnOk(wxCommandEvent& evt) {
    auto readClamped = [&](const char* id, int lo, int hi) {
        long v = 0;
        XRCCTRL(*this, id, wxTextCtrl)->GetValue().ToLong(&v);
        if (v < lo) v = lo;
        if (v > hi) v = hi;
        return static_cast<int>(v);
    };

    m_prefs.Height = readClamped("ID_EDIT_HEIGHT", kMinHeight, kMaxHeight);
    m_prefs.Width  = readClamped("ID_EDIT_WIDTH",  kMinWidth,  kMaxWidth);
    const int mineCap = std::min(999, (m_prefs.Height - 1) * (m_prefs.Width - 1));
    m_prefs.Mines  = readClamped("ID_EDIT_MINES", 10, mineCap);

    evt.Skip(); // let the dialog close with wxID_OK
}

} // namespace wxmine
