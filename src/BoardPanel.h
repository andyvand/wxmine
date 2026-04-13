#pragma once

#include <wx/panel.h>
#include <wx/timer.h>

namespace wxmine {

class GameCore;
class Renderer;

// Owns the wxTimer and forwards mouse events to GameCore. Painting is
// delegated to Renderer in OnPaint via a wxBufferedPaintDC for flicker-free
// blits.
class BoardPanel : public wxPanel {
public:
    BoardPanel(wxWindow* parent, GameCore& game, Renderer& renderer);

    // Resizes the panel to fit the current board (called by GameCore via
    // its Resize callback). Returns the new size so MainFrame can resize too.
    wxSize ResizeForBoard(int boardW, int boardH);

    // Convenience repaint helpers wired into GameCore callbacks.
    void RepaintCell(int x, int y);
    void RepaintGrid();
    void RepaintButton();
    void RepaintBombCount();
    void RepaintTime();

    // Click handler for the smiley/restart button hit-test.
    bool HitSmiley(const wxPoint& p) const;

private:
    void OnPaint(wxPaintEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnRightDown(wxMouseEvent& evt);
    void OnRightUp(wxMouseEvent& evt);
    void OnMiddleDown(wxMouseEvent& evt);
    void OnMiddleUp(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnTimer(wxTimerEvent& evt);

    void StartOrStopTimerFromGame();

    int  XCellFromPx(int px) const;
    int  YCellFromPx(int py) const;

    GameCore& m_game;
    Renderer& m_renderer;
    wxTimer   m_timer;

    // Tracks whether we entered "smiley press" mode on left-down so we can
    // restart the game on left-up.
    bool m_smileyPressed = false;

    // Was the left button down when this right-click came in? Used to
    // implement the simultaneous left+right chord.
    bool m_leftDownWhenRight = false;
};

} // namespace wxmine
