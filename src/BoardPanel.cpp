#include "BoardPanel.h"

#include <algorithm>

#include <wx/dcbuffer.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

#include "GameConstants.h"
#include "GameCore.h"
#include "Renderer.h"

namespace wxmine {

namespace {
constexpr int    kSmileyTimerId = 4242;
constexpr double kMinScale      = 2.0;
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
    Bind(wxEVT_SIZE,          &BoardPanel::OnSize,        this);
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
    const wxSize nativeSize = m_renderer.ComputePanelSize(boardW, boardH);
    const wxSize size(static_cast<int>(nativeSize.x * kMinScale),
                      static_cast<int>(nativeSize.y * kMinScale));
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

void BoardPanel::ComputeLayout() {
    const wxSize nativeSize = m_renderer.ComputePanelSize(
        m_game.BoardWidth(), m_game.BoardHeight());
    const wxSize clientSize = GetClientSize();
    if (nativeSize.x <= 0 || nativeSize.y <= 0) {
        m_scale = kMinScale;
        m_offsetX = m_offsetY = 0;
        return;
    }
    const double sx = static_cast<double>(clientSize.x) / nativeSize.x;
    const double sy = static_cast<double>(clientSize.y) / nativeSize.y;
    m_scale = std::max(kMinScale, std::min(sx, sy));
    m_offsetX = static_cast<int>((clientSize.x - nativeSize.x * m_scale) / 2.0);
    m_offsetY = static_cast<int>((clientSize.y - nativeSize.y * m_scale) / 2.0);
    if (m_offsetX < 0) m_offsetX = 0;
    if (m_offsetY < 0) m_offsetY = 0;
}

void BoardPanel::OnSize(wxSizeEvent& evt) {
    ComputeLayout();
    Refresh(false);
    evt.Skip();
}

void BoardPanel::OnPaint(wxPaintEvent& /*evt*/) {
    wxAutoBufferedPaintDC dc(this);
    ComputeLayout();

    // Fill the whole panel with the classic gray so any letterbox area
    // surrounding the scaled board matches the sprite background.
    dc.SetBackground(wxBrush(wxColour(192, 192, 192)));
    dc.Clear();

    const wxSize nativeSize = m_renderer.ComputePanelSize(
        m_game.BoardWidth(), m_game.BoardHeight());

    if (m_scale == 1.0) {
        dc.SetDeviceOrigin(m_offsetX, m_offsetY);
        m_renderer.DrawScreen(dc, m_game);
        dc.SetDeviceOrigin(0, 0);
        return;
    }

    // Render at native resolution to an off-screen bitmap, then scale up
    // with nearest-neighbor for crisp pixel-art enlargement.
    wxBitmap buffer(nativeSize.x, nativeSize.y);
    {
        wxMemoryDC mem(buffer);
        m_renderer.DrawScreen(mem, m_game);
    }
    wxImage img = buffer.ConvertToImage();
    img.Rescale(static_cast<int>(nativeSize.x * m_scale),
                static_cast<int>(nativeSize.y * m_scale),
                wxIMAGE_QUALITY_NEAREST);
    dc.DrawBitmap(wxBitmap(img), m_offsetX, m_offsetY, false);
}

int BoardPanel::XCellFromPx(int px) const {
    const int local = static_cast<int>((px - m_offsetX) / m_scale);
    return (local - (dxGridOff - dxBlk)) >> 4;
}
int BoardPanel::YCellFromPx(int py) const {
    const int local = static_cast<int>((py - m_offsetY) / m_scale);
    return (local - (dyGridOff - dyBlk)) >> 4;
}

bool BoardPanel::HitSmiley(const wxPoint& p) const {
    const int dxWindow = dxBlk * m_game.BoardWidth() + dxGridOff + dxRightSpace;
    const int x = (dxWindow - dxButton) >> 1;
    const int lx = static_cast<int>((p.x - m_offsetX) / m_scale);
    const int ly = static_cast<int>((p.y - m_offsetY) / m_scale);
    return lx >= x && lx < x + dxButton
        && ly >= dyTopLed && ly < dyTopLed + dyButton;
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
