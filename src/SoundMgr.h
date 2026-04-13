#pragma once

#include <memory>
#include <vector>

#include <wx/string.h>

#include "GameConstants.h"

class wxSound;

namespace wxmine {

struct Preferences;

// Replaces sound.c. Owns three wxSound instances loaded from the embedded
// WAV data and plays them asynchronously when the user preference is on.
class SoundMgr {
public:
    SoundMgr();
    ~SoundMgr();

    // Called once at startup after wxXmlResource has been initialised.
    // Returns fsoundOn (3) on success, fsoundOff (2) if audio isn't usable.
    int Init();

    // Stop any currently playing sound (sound.c::EndTunes).
    void Stop();

    // Play a tune iff Preferences.fSound == fsoundOn (sound.c::PlayTune).
    void Play(Tune tune, const Preferences& prefs);

private:
    std::unique_ptr<wxSound> m_tick;
    std::unique_ptr<wxSound> m_win;
    std::unique_ptr<wxSound> m_lose;
    // Temp wav files extracted from the embedded .xrs at startup; deleted in
    // ~SoundMgr. wxSound on macOS only implements Create(filename), not
    // Create(size, data), so we materialise the WAVs to disk once.
    std::vector<wxString> m_tempFiles;
    bool m_initialised = false;
};

} // namespace wxmine
