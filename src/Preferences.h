#pragma once

#include <wx/string.h>

#include "GameConstants.h"

namespace wxmine {

// Ported from pref.h. Holds user-visible preferences plus the persisted
// best-time table. Registry-equivalent persistence is provided by wxConfig.
struct Preferences {
    int      wGameType = kGameBegin;  // Difficulty (0..3)
    int      Mines     = 10;
    int      Height    = 9;
    int      Width     = 9;
    int      xWindow   = 80;
    int      yWindow   = 80;
    int      fSound    = 0;           // 0=never-chose, 2=off, 3=on (matches fsoundOn/Off)
    bool     fMark     = true;        // question-mark flag enabled
    bool     fTick     = false;
    int      fMenu     = 0;           // fmenuAlwaysOn
    bool     fColor    = true;
    int      rgTime[3] = {999, 999, 999};
    wxString szBegin;
    wxString szInter;
    wxString szExpert;

    // Reads all fields from wxConfig (equivalent of pref.c::ReadPreferences).
    void Load();

    // Writes all fields to wxConfig (equivalent of pref.c::WritePreferences).
    void Save() const;

    // Resets best-times/names to their defaults.
    void ResetBestTimes();

    // Maximum number of characters for a best-time name (cchNameMax in pref.h).
    static constexpr int kNameMax = 32;
};

} // namespace wxmine
