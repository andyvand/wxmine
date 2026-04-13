#include <wx/app.h>
#include <wx/config.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/xrc/xmlres.h>

#include "MainFrame.h"
#include "Resources.h"

namespace wxmine {

class WinMineApp : public wxApp {
public:
    bool OnInit() override {
        if (!wxApp::OnInit()) return false;

        SetAppName("wxmine");
        SetVendorName("wxmine");
        wxConfigBase::Set(new wxConfig("wxmine", "wxmine"));

        // wxImage handlers are needed before LoadFile() can decode the
        // bundled PNGs in the renderer.
        wxImage::AddHandler(new wxPNGHandler);

        // Memory FS lets us mount the bin2c-embedded winmine.xrs blob as
        // "memory:winmine.xrs"; archive FS lets paths of the form
        //   memory:winmine.xrs#zip:assets/blocks.png
        // resolve into individual zip entries. wxXmlResource::Load() also
        // relies on the archive handler to walk the .xrs entries.
        wxFileSystem::AddHandler(new wxMemoryFSHandler);
        wxFileSystem::AddHandler(new wxArchiveFSHandler);

        RegisterEmbeddedXrs();

        wxXmlResource::Get()->InitAllHandlers();

        const wxString xrs = XrsUrl();
        if (!wxXmlResource::Get()->Load(xrs)) {
            wxMessageBox(
                wxString::Format("Failed to load resources from:\n%s", xrs),
                "Minesweeper", wxICON_ERROR | wxOK);
            return false;
        }

        auto* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

} // namespace wxmine

wxIMPLEMENT_APP(wxmine::WinMineApp);
