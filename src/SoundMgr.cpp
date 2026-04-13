#include "SoundMgr.h"

#include <wx/sound.h>
#include <wx/filesys.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/wfstream.h>

#include "Preferences.h"
#include "Resources.h"

namespace wxmine {

namespace {

// Read a wav blob out of the embedded winmine.xrs and write it to a temp
// file, returning the absolute path. Empty path means failure.
//
// We can't go through wxSound::Create(size, data) because that overload is
// "not implemented" on the wxOSX core backend (asserts at sound.cpp:128).
// Materialising the bytes to disk lets us use the always-available
// Create(filename) path on every platform.
wxString ExtractToTempFile(const wxString& innerPath, const wxString& basename) {
    wxFileSystem fs;
    std::unique_ptr<wxFSFile> src(fs.OpenFile(AssetUrl(innerPath)));
    if (!src) return {};

    wxInputStream* in = src->GetStream();
    if (!in) return {};

    wxFileName outFn(wxStandardPaths::Get().GetTempDir(), basename);
    // Make the path session-unique so multiple instances don't stomp on each
    // other and stale files from a previous run can't be mistaken for ours.
    outFn.SetName(wxString::Format("wxmine-%ld-%s",
                                   static_cast<long>(wxGetProcessId()),
                                   outFn.GetName()));

    {
        wxFileOutputStream out(outFn.GetFullPath());
        if (!out.IsOk()) return {};
        out.Write(*in);
        if (!out.IsOk()) return {};
    }
    return outFn.GetFullPath();
}

std::unique_ptr<wxSound> LoadWavFromTemp(const wxString& path) {
    if (path.empty()) return nullptr;
    auto sound = std::make_unique<wxSound>();
    if (!sound->Create(path)) return nullptr;
    return sound;
}

} // namespace

SoundMgr::SoundMgr() = default;

SoundMgr::~SoundMgr() {
    // Free the wxSound objects before deleting their backing files, in case
    // any backend keeps the file mapped.
    m_tick.reset();
    m_win.reset();
    m_lose.reset();
    for (const auto& path : m_tempFiles) {
        wxRemoveFile(path);
    }
}

int SoundMgr::Init() {
    const wxString tickPath = ExtractToTempFile("assets/tick.wav",    "tick.wav");
    const wxString winPath  = ExtractToTempFile("assets/win.wav",     "win.wav");
    const wxString losePath = ExtractToTempFile("assets/explode.wav", "explode.wav");

    if (!tickPath.empty()) m_tempFiles.push_back(tickPath);
    if (!winPath.empty())  m_tempFiles.push_back(winPath);
    if (!losePath.empty()) m_tempFiles.push_back(losePath);

    m_tick = LoadWavFromTemp(tickPath);
    m_win  = LoadWavFromTemp(winPath);
    m_lose = LoadWavFromTemp(losePath);

    if (!m_tick || !m_win || !m_lose) {
        m_initialised = false;
        return 2; // fsoundOff
    }
    m_initialised = true;
    return 3;     // fsoundOn
}

void SoundMgr::Stop() {
    wxSound::Stop();
}

void SoundMgr::Play(Tune tune, const Preferences& prefs) {
    if (!m_initialised || prefs.fSound != 3) {
        return;
    }
    wxSound* s = nullptr;
    switch (tune) {
        case kTuneTick: s = m_tick.get(); break;
        case kTuneWin:  s = m_win.get();  break;
        case kTuneLose: s = m_lose.get(); break;
    }
    if (s) {
        s->Play(wxSOUND_ASYNC);
    }
}

} // namespace wxmine
