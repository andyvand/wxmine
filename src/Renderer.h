#pragma once

#include <wx/bitmap.h>
#include <wx/dc.h>
#include <wx/string.h>

#include "GameConstants.h"

namespace wxmine {

class GameCore;
struct Preferences;

// Replaces grafix.c. Owns the sprite-sheet bitmaps (blocks/LEDs/button) and
// knows how to draw the entire game window into a wxDC.
//
// Each sprite sheet is a single vertical strip whose tiles correspond 1:1
// with the iBlk*/iLed*/iButton* index constants in GameConstants.h. The
// original Win32 code stored sprites bottom-up in a packed DIB; when
// converted to a top-down PNG by sips, sprite index i now lives at
// pixel-y = (sheetHeight - (i+1)*tileHeight). Renderer hides that detail.
class Renderer {
public:
    Renderer();

    // Load all bitmaps from the wxrc-embedded "memory:" filesystem. Must be
    // called once after wxXmlResource::InitAllHandlers(). Returns false if
    // any sprite sheet failed to load.
    bool Load();

    // Re-load with the opposite color/B&W variant when the user toggles
    // Game → Color (replaces FreeBitmaps()/FLoadBitmaps() in grafix.c).
    bool Reload(bool color);

    // Whole-window paint. Called from BoardPanel::OnPaint with a buffered DC.
    void DrawScreen(wxDC& dc, const GameCore& game);

    // Pixel size of the play surface for the current game (used by
    // BoardPanel/MainFrame to size themselves). Mirrors the dxWindow/dyWindow
    // arithmetic in winmine.c::AdjustWindow.
    wxSize ComputePanelSize(int boardW, int boardH) const;

private:
    // Tile blits — each one Blit()s a single sub-rect of the sheet to dc.
    void DrawBlk(wxDC& dc, int x, int y, int blkIndex);
    void DrawGrid(wxDC& dc, const GameCore& game);
    void DrawButton(wxDC& dc, int boardW, int state);
    void DrawTime(wxDC& dc, int boardW, int sec);
    void DrawBombCount(wxDC& dc, int bombsLeft);
    void DrawLed(wxDC& dc, int x, int ledIndex);
    void DrawBackground(wxDC& dc, int boardW, int boardH);
    void DrawBorder(wxDC& dc, int x1, int y1, int x2, int y2,
                    int width, int variant);

    // Helpers for the bottom-up-vs-top-down tile lookup.
    wxRect TileRect(const wxBitmap& sheet, int tileW, int tileH, int index) const;

    bool m_color = true;

    wxBitmap m_blocks;   // 16 tiles of 16×16
    wxBitmap m_led;      // 12 tiles of 13×23
    wxBitmap m_button;   // 5 tiles of 24×24
};

} // namespace wxmine
