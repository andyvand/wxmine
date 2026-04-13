#include "Resources.h"

#include <wx/fs_mem.h>

// bin2c-generated header: defines `const unsigned char winmine_xrs[]` with
// the contents of the wxrc-built zip. This file is the single translation
// unit that includes it (the array is defined, not just declared).
#include "winmine_xrs.h"

namespace wxmine {

namespace {
constexpr const char* kMemoryName = "winmine.xrs";
}

void RegisterEmbeddedXrs() {
    // Idempotent: re-registering the same memory file would assert in wx.
    static bool registered = false;
    if (registered) return;
    wxMemoryFSHandler::AddFileWithMimeType(
        kMemoryName,
        winmine_xrs,
        sizeof(winmine_xrs),
        "application/zip");
    registered = true;
}

wxString XrsUrl() {
    return wxString("memory:") + kMemoryName;
}

wxString AssetUrl(const wxString& innerPath) {
    return XrsUrl() + "#zip:" + innerPath;
}

} // namespace wxmine
