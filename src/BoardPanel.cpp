#include "BoardPanel.h"

#include <wx/dcbuffer.h>

#include "GameCore.h"
#include "Renderer.h"

namespace wxmine {

namespace {
constexpr int kSmileyTimerId = 4242;
}

BoardPanel::BoardPanel(wxWindow* parent, GameCore& game, Renderer& renderer)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      m_game(game),
      m_renderer(renderer),
      m_timer(this, kSmileyTimerId)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);

    Bind(wxEVT_PAINT,         &BoardPanel::OnPaint,       this);
    Bind(wxEVT_LEFT_DOWN,     &BoardPanel::OnLeftDown,    this);
    Bind(wxEVT_LEFT_UP,       &BoardPanel::OnLeftUp,      this);
    Bind(wxEVT_RIGHT_DOWN,    &BoardPanel::OnRightDown,   this);
    Bind(wxEVT_RIGHT_UP,      &BoardPanel::OnRightUp,     this);
    Bind(wxEVT_MIDDLE_DOWN,   &BoardPanel::OnMiddleDown,  this);
    Bind(wxEVT_MIDDLE_UP,     &BoardPanel::OnMiddleUp,    this);
    Bind(wxEVT_MOTION,        &BoardPanel::OnMotion,      this);
    Bind(wxEVT_TIMER,         &BoardPanel::OnTimer,       this, kSmileyTimerId);
}

wxSize BoardPanel::ResizeForBoard(int boardW, int boardH) {
    const wxSize size = m_renderer.ComputePanelSize(boardW, boardH);
    SetMinSize(size);
    SetSize(size);
    Refresh();
    return size;
}

void BoardPanel::RepaintCell(int /*x*/, int /*y*/) {
    // Could compute a tight rect, but a full repaint is cheap on small boards
    // and avoids subtleties with the bevels around the grid.
    Refresh(false);
}

void BoardPanel::RepaintGrid()       { Refresh(false); }
void BoardPanel::RepaintButton()     { Refresh(false); }
void BoardPanel::RepaintBombCount()  { Refresh(false); }
void BoardPanel::RepaintTime()       { Refresh(false); StartOrStopTimerFromGame(); }

void BoardPanel::OnPaint(wxPaintEvent& /*evt*/) {
    wxAutoBufferedPaintDC dc(this);
    m_renderer.DrawScreen(dc, m_game);
}

int BoardPanel::XCellFromPx(int px) const {
    return (px - (dxGridOff - dxBlk)) >> 4;
}
int BoardPanel::YCellFromPx(int py) const {
    return (py - (dyGridOff - dyBlk)) >> 4;
}

bool BoardPanel::HitSmiley(const wxPoint& p) const {
    const int dxWindow = dxBlk * m_game.BoardWidth() + dxGridOff + dxRightSpace;
    const int x = (dxWindow - dxButton) >> 1;
    return p.x >= x && p.x < x + dxButton
        && p.y >= dyTopLed && p.y < dyTopLed + dyButton;
}

void BoardPanel::OnLeftDown(wxMouseEvent& evt) {
    if (HitSmiley(evt.GetPosition())) {
        m_smileyPressed = true;
        m_game.SetButtonCaution();
        CaptureMouse();
        return;
    }
    if (!HasCapture()) CaptureMouse();
    m_game.OnLeftDown(XCellFromPx(evt.GetX()),
                      YCellFromPx(evt.GetY()),
                      evt.RightIsDown() || evt.ShiftDown());
}

void BoardPanel::OnLeftUp(wxMouseEvent& evt) {
    if (HasCapture()) ReleaseMouse();
    if (m_smileyPressed) {
        m_smileyPressed = false;
        if (HitSmiley(evt.GetPosition())) {
            m_game.Reset();
        } else {
            m_game.SetButtonHappy();
        }
        return;
    }
    m_game.OnButton1Up();
    StartOrStopTimerFromGame();
}

void BoardPanel::OnRightDown(wxMouseEvent& evt) {
    m_game.OnRightDown(XCellFromPx(evt.GetX()),
                       YCellFromPx(evt.GetY()));
}

void BoardPanel::OnRightUp(wxMouseEvent& /*evt*/) {
    m_game.OnButton1Up();
}

void BoardPanel::OnMiddleDown(wxMouseEvent& evt) {
    if (!HasCapture()) CaptureMouse();
    m_game.OnMiddleDown(XCellFromPx(evt.GetX()),
                        YCellFromPx(evt.GetY()));
}

void BoardPanel::OnMiddleUp(wxMouseEvent& /*evt*/) {
    if (HasCapture()) ReleaseMouse();
    m_game.OnButton1Up();
    StartOrStopTimerFromGame();
}

void BoardPanel::OnMotion(wxMouseEvent& evt) {
    if (evt.LeftIsDown() || evt.MiddleIsDown()) {
        m_game.TrackMouse(XCellFromPx(evt.GetX()),
                          YCellFromPx(evt.GetY()));
    }
}

void BoardPanel::OnTimer(wxTimerEvent& /*evt*/) {
    m_game.Tick();
}

void BoardPanel::StartOrStopTimerFromGame() {
    const bool wantRunning = m_game.IsTimerRunning();
    if (wantRunning && !m_timer.IsRunning()) {
        m_timer.Start(1000);
    } else if (!wantRunning && m_timer.IsRunning()) {
        m_timer.Stop();
    }
}

} // namespace wxmine
