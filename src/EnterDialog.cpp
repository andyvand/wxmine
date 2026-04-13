#include "EnterDialog.h"

#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

#include "GameConstants.h"
#include "Preferences.h"

namespace wxmine {

namespace {

// Strings from the original strings.inc — we keep the literal text so the
// dialog feels identical to classic Minesweeper. Indexed by GameType value.
const wxString kBestPrompt[3] = {
    "You have the fastest time\nfor beginner level.\nPlease enter your name.",
    "You have the fastest time\nfor intermediate level.\nPlease enter your name.",
    "You have the fastest time\nfor expert level.\nPlease enter your name.",
};

wxString& NameSlot(Preferences& p, int level) {
    if (level == kGameInter)  return p.szInter;
    if (level == kGameExpert) return p.szExpert;
    return p.szBegin;
}

} // namespace

EnterDialog::EnterDialog(wxWindow* parent, Preferences& prefs, int level)
    : m_prefs(prefs), m_level(level)
{
    wxXmlResource::Get()->LoadDialog(this, parent, "ID_DLG_ENTER");

    XRCCTRL(*this, "ID_TEXT_BEST", wxStaticText)
        ->SetLabel(kBestPrompt[std::min(2, std::max(0, level))]);

    auto* edit = XRCCTRL(*this, "ID_EDIT_NAME", wxTextCtrl);
    edit->SetMaxLength(Preferences::kNameMax);
    edit->SetValue(NameSlot(prefs, level));
    edit->SetFocus();
    edit->SelectAll();

    Bind(wxEVT_BUTTON, &EnterDialog::OnOk, this, wxID_OK);
    Layout();
    Fit();
}

void EnterDialog::OnOk(wxCommandEvent& evt) {
    NameSlot(m_prefs, m_level) =
        XRCCTRL(*this, "ID_EDIT_NAME", wxTextCtrl)->GetValue();
    evt.Skip();
}

} // namespace wxmine
