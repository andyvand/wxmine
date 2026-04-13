#include "Preferences.h"

#include <wx/config.h>

namespace wxmine {

namespace {

// Clamp val into [lo, hi].
inline int clampInt(int val, int lo, int hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

// Default player name for unset best-time entries. Matches ID_NAME_DEFAULT
// ("Anonymous") from the original strings.inc.
const wxString kDefaultName = "Anonymous";

} // namespace

void Preferences::Load() {
    wxConfigBase* cfg = wxConfigBase::Get();

    // Read integers with clamping — mirrors pref.c::ReadInt.
    long v;
    wGameType = cfg->Read("Difficulty", &v) ? clampInt((int)v, kGameBegin, kGameExpert + 1)
                                            : kGameBegin;
    Height    = cfg->Read("Height", &v) ? clampInt((int)v, kMinHeight, kMaxHeight) : kMinHeight;
    Width     = cfg->Read("Width",  &v) ? clampInt((int)v, kMinWidth,  kMaxWidth)  : kMinWidth;
    Mines     = cfg->Read("Mines",  &v) ? clampInt((int)v, 10, 999) : 10;
    xWindow   = cfg->Read("Xpos",   &v) ? clampInt((int)v, 0, 4096) : 80;
    yWindow   = cfg->Read("Ypos",   &v) ? clampInt((int)v, 0, 4096) : 80;
    fSound    = cfg->Read("Sound",  &v) ? clampInt((int)v, 0, 3)    : 0;
    fMark     = cfg->Read("Mark",   &v) ? (v != 0) : true;
    fTick     = cfg->Read("Tick",   &v) ? (v != 0) : false;
    fMenu     = cfg->Read("Menu",   &v) ? clampInt((int)v, 0, 2)    : 0;
    fColor    = cfg->Read("Color",  &v) ? (v != 0) : true;

    rgTime[kGameBegin]  = cfg->Read("Time1", &v) ? clampInt((int)v, 0, 999) : 999;
    rgTime[kGameInter]  = cfg->Read("Time2", &v) ? clampInt((int)v, 0, 999) : 999;
    rgTime[kGameExpert] = cfg->Read("Time3", &v) ? clampInt((int)v, 0, 999) : 999;

    wxString s;
    szBegin  = cfg->Read("Name1", &s) ? s : kDefaultName;
    szInter  = cfg->Read("Name2", &s) ? s : kDefaultName;
    szExpert = cfg->Read("Name3", &s) ? s : kDefaultName;
}

void Preferences::Save() const {
    wxConfigBase* cfg = wxConfigBase::Get();

    cfg->Write("Difficulty", (long)wGameType);
    cfg->Write("Height",     (long)Height);
    cfg->Write("Width",      (long)Width);
    cfg->Write("Mines",      (long)Mines);
    cfg->Write("Xpos",       (long)xWindow);
    cfg->Write("Ypos",       (long)yWindow);
    cfg->Write("Sound",      (long)fSound);
    cfg->Write("Mark",       (long)(fMark ? 1 : 0));
    cfg->Write("Tick",       (long)(fTick ? 1 : 0));
    cfg->Write("Menu",       (long)fMenu);
    cfg->Write("Color",      (long)(fColor ? 1 : 0));

    cfg->Write("Time1", (long)rgTime[kGameBegin]);
    cfg->Write("Time2", (long)rgTime[kGameInter]);
    cfg->Write("Time3", (long)rgTime[kGameExpert]);

    cfg->Write("Name1", szBegin);
    cfg->Write("Name2", szInter);
    cfg->Write("Name3", szExpert);

    cfg->Write("AlreadyPlayed", 1L);

    cfg->Flush();
}

void Preferences::ResetBestTimes() {
    rgTime[kGameBegin]  = 999;
    rgTime[kGameInter]  = 999;
    rgTime[kGameExpert] = 999;
    szBegin  = kDefaultName;
    szInter  = kDefaultName;
    szExpert = kDefaultName;
}

} // namespace wxmine
