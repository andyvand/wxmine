#pragma once

#include <wx/frame.h>

#include "Preferences.h"
#include "SoundMgr.h"

namespace wxmine {

class GameCore;
class Renderer;
class BoardPanel;

// Top-level window. Owns the game state, renderer, sound manager, and
// embeds a BoardPanel as the only child. Replaces winmine.c::MainWndProc and
// the WinMain plumbing.
class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame() override;

private:
    // Menu handlers (mirror the WM_COMMAND switch in winmine.c).
    void OnNew(wxCommandEvent&);
    void OnSetDifficulty(wxCommandEvent& evt);
    void OnCustom(wxCommandEvent&);
    void OnToggleMark(wxCommandEvent&);
    void OnToggleColor(wxCommandEvent&);
    void OnToggleSound(wxCommandEvent&);
    void OnBest(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);

    void OnIconize(wxIconizeEvent& evt);
    void OnClose(wxCloseEvent& evt);

    // Refreshes menu check-marks to match Preferences (winmine.c::FixMenus).
    void FixMenus();

    // Resizes the frame around a fresh BoardPanel size.
    void ResizeForBoard(int boardW, int boardH);

    Preferences m_prefs;
    SoundMgr    m_sound;

    Renderer*   m_renderer = nullptr;  // owned via std::unique_ptr in cpp
    GameCore*   m_game     = nullptr;  // owned via std::unique_ptr in cpp
    BoardPanel* m_panel    = nullptr;  // child of frame, deleted by wx

    struct Owned;
    Owned* m_owned = nullptr;
};

} // namespace wxmine
