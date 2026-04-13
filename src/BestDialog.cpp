#include "BestDialog.h"

#include <wx/stattext.h>
#include <wx/xrc/xmlres.h>

#include "GameConstants.h"
#include "Preferences.h"

namespace wxmine {

BestDialog::BestDialog(wxWindow* parent, Preferences& prefs)
    : m_prefs(prefs)
{
    wxXmlResource::Get()->LoadDialog(this, parent, "ID_DLG_BEST");
    Refresh();
    Bind(wxEVT_BUTTON, &BestDialog::OnReset, this,
         XRCID("ID_BTN_RESET"));
}

void BestDialog::Refresh() {
    auto setRow = [&](const char* timeId, const char* nameId,
                      int sec, const wxString& name) {
        XRCCTRL(*this, timeId, wxStaticText)
            ->SetLabel(wxString::Format("%d seconds", sec));
        XRCCTRL(*this, nameId, wxStaticText)->SetLabel(name);
    };
    setRow("ID_TIME_BEGIN",  "ID_NAME_BEGIN",  m_prefs.rgTime[kGameBegin],  m_prefs.szBegin);
    setRow("ID_TIME_INTER",  "ID_NAME_INTER",  m_prefs.rgTime[kGameInter],  m_prefs.szInter);
    setRow("ID_TIME_EXPERT", "ID_NAME_EXPERT", m_prefs.rgTime[kGameExpert], m_prefs.szExpert);
    Layout();
}

void BestDialog::OnReset(wxCommandEvent& /*evt*/) {
    m_prefs.ResetBestTimes();
    Refresh();
}

} // namespace wxmine
