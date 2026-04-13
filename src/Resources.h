#pragma once

#include <wx/string.h>

namespace wxmine {

// Mount the bin2c-embedded winmine.xrs blob into the wxMemoryFSHandler so
// that it becomes accessible via the "memory:winmine.xrs" wxFileSystem URL.
// Must be called once after wxMemoryFSHandler has been registered, and
// before any code attempts to open AssetUrl(...) or load XML resources.
void RegisterEmbeddedXrs();

// wxFileSystem URL of the embedded winmine.xrs zip archive itself.
// Suitable for wxXmlResource::Load().
wxString XrsUrl();

// wxFileSystem URL of an asset inside the embedded winmine.xrs.
// Example: AssetUrl("assets/blocks.png") -> "memory:winmine.xrs#zip:assets/blocks.png"
// The wxArchiveFSHandler must be registered for this to resolve.
wxString AssetUrl(const wxString& innerPath);

} // namespace wxmine
