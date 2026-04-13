#include "Renderer.h"

#include <wx/dcmemory.h>
#include <wx/image.h>
#include <wx/filesys.h>
#include <wx/mstream.h>
#include <wx/pen.h>

#include "GameCore.h"
#include "Preferences.h"
#include "Resources.h"

namespace wxmine {

namespace {

// Read an asset out of the bundled winmine.xrs (via wxArchiveFSHandler) and
// decode it as a PNG into a wxBitmap. Returns an invalid bitmap on failure.
wxBitmap LoadXrsPng(const wxString& innerPath) {
    wxFileSystem fs;
    std::unique_ptr<wxFSFile> file(fs.OpenFile(AssetUrl(innerPath)));
    if (!file) {
        return wxBitmap();
    }
    wxImage img;
    if (!img.LoadFile(*file->GetStream(), wxBITMAP_TYPE_PNG)) {
        return wxBitmap();
    }
    return wxBitmap(img);
}

} // namespace

Renderer::Renderer() = default;

bool Renderer::Load() {
    return Reload(m_color);
}

bool Renderer::Reload(bool color) {
    m_color = color;
    const char* blocks = color ? "assets/blocks.png" : "assets/blocksbw.png";
    const char* led    = color ? "assets/led.png"    : "assets/ledbw.png";
    const char* button = color ? "assets/button.png" : "assets/buttonbw.png";

    m_blocks = LoadXrsPng(blocks);
    m_led    = LoadXrsPng(led);
    m_button = LoadXrsPng(button);

    return m_blocks.IsOk() && m_led.IsOk() && m_button.IsOk();
}

wxRect Renderer::TileRect(const wxBitmap& sheet, int tileW, int tileH,
                          int index) const {
    // The original .bmp files are bottom-up Windows DIBs: sprite 0 occupies
    // the BOTTOM strip, sprite 1 above it, etc. After sips converts to a
    // top-down PNG, sprite 0 ends up at the bottom of the image.
    const int y = sheet.GetHeight() - (index + 1) * tileH;
    return wxRect(0, y, tileW, tileH);
}

void Renderer::DrawBlk(wxDC& dc, int x, int y, int blkIndex) {
    const wxRect src = TileRect(m_blocks, dxBlk, dyBlk, blkIndex);
    wxMemoryDC mem(m_blocks);
    dc.Blit((x << 4) + (dxGridOff - dxBlk),
            (y << 4) + (dyGridOff - dyBlk),
            dxBlk, dyBlk,
            &mem, src.x, src.y);
}

void Renderer::DrawGrid(wxDC& dc, const GameCore& game) {
    wxMemoryDC mem(m_blocks);
    int dy = dyGridOff;
    for (int y = 1; y <= game.BoardHeight(); ++y, dy += dyBlk) {
        int dx = dxGridOff;
        for (int x = 1; x <= game.BoardWidth(); ++x, dx += dxBlk) {
            const int blkIndex = game.VisibleBlk(x, y);
            const wxRect src = TileRect(m_blocks, dxBlk, dyBlk, blkIndex);
            dc.Blit(dx, dy, dxBlk, dyBlk, &mem, src.x, src.y);
        }
    }
}

void Renderer::DrawLed(wxDC& dc, int x, int ledIndex) {
    const wxRect src = TileRect(m_led, dxLed, dyLed, ledIndex);
    wxMemoryDC mem(m_led);
    dc.Blit(x, dyTopLed, dxLed, dyLed, &mem, src.x, src.y);
}

void Renderer::DrawBombCount(wxDC& dc, int bombsLeft) {
    int iLed, cBombs;
    if (bombsLeft < 0) {
        iLed = iLedNegative;
        cBombs = (-bombsLeft) % 100;
    } else {
        iLed = bombsLeft / 100;
        cBombs = bombsLeft % 100;
    }
    DrawLed(dc, dxLeftBomb,                   iLed);
    DrawLed(dc, dxLeftBomb + dxLed,           cBombs / 10);
    DrawLed(dc, dxLeftBomb + 2 * dxLed,       cBombs % 10);
}

void Renderer::DrawTime(wxDC& dc, int boardW, int sec) {
    const int dxWindow = dxBlk * boardW + dxGridOff + dxRightSpace;
    int t = sec;
    DrawLed(dc, dxWindow - (dxRightTime + 3 * dxLed), t / 100);
    DrawLed(dc, dxWindow - (dxRightTime + 2 * dxLed), (t %= 100) / 10);
    DrawLed(dc, dxWindow - (dxRightTime + 1 * dxLed), t % 10);
}

void Renderer::DrawButton(wxDC& dc, int boardW, int state) {
    const int dxWindow = dxBlk * boardW + dxGridOff + dxRightSpace;
    const wxRect src = TileRect(m_button, dxButton, dyButton, state);
    wxMemoryDC mem(m_button);
    dc.Blit((dxWindow - dxButton) >> 1, dyTopLed,
            dxButton, dyButton,
            &mem, src.x, src.y);
}

void Renderer::DrawBorder(wxDC& dc, int x1, int y1, int x2, int y2,
                          int width, int variant) {
    // variant: 0 = top-white, bottom-gray (sunken)
    //          1 = top-gray, bottom-white (raised)
    //          2 = solid white box
    //          3 = solid gray box
    const wxColour kGray(128, 128, 128);
    const wxColour kWhite(255, 255, 255);

    auto pickFirst = [&](int v) -> wxColour {
        if (v & 1) return kWhite;
        return kGray;
    };
    auto pickSecond = [&](int v) -> wxColour {
        if (v >= 2) return pickFirst(v); // single-color box
        return pickFirst(v ^ 1);
    };

    int i = 0;
    dc.SetPen(wxPen(pickFirst(variant)));
    while (i++ < width) {
        --y2;
        dc.DrawLine(x1, y2, x1, y1);
        dc.DrawLine(x1, y1, x2, y1);
        ++x1;
        --x2;
        ++y1;
    }

    dc.SetPen(wxPen(pickSecond(variant)));
    while (--i) {
        ++y2;
        dc.DrawLine(x1, y2, x2, y2);
        dc.DrawLine(x2, y2, x2, y1);
        --x1;
        ++x2;
        --y1;
    }
}

void Renderer::DrawBackground(wxDC& dc, int boardW, int boardH) {
    const int dxWindow = dxBlk * boardW + dxGridOff + dxRightSpace;
    const int dyWindow = dyBlk * boardH + dyGridOff + dyBottomSpace;

    // Fill with the classic light-gray panel color.
    dc.SetBackground(wxBrush(wxColour(192, 192, 192)));
    dc.Clear();

    int x = dxWindow - 1;
    int y = dyWindow - 1;
    DrawBorder(dc, 0, 0, x, y, 3, 1);                        // outer raised

    x -= (dxRightSpace - 3);
    y -= (dyBottomSpace - 3);
    DrawBorder(dc, dxGridOff - 3, dyGridOff - 3, x, y, 3, 0); // grid sunken
    DrawBorder(dc, dxGridOff - 3, dyTopSpace - 3, x,
               dyTopLed + dyLed + (dyBottomSpace - 6), 2, 0); // status sunken

    // Bomb-count LED frame.
    x = dxLeftBomb + dxLed * 3;
    y = dyTopLed + dyLed;
    DrawBorder(dc, dxLeftBomb - 1, dyTopLed - 1, x, y, 1, 0);

    // Time LED frame.
    int tx = dxWindow - (dxRightTime + 3 * dxLed + 1);
    DrawBorder(dc, tx, dyTopLed - 1, tx + (dxLed * 3 + 1), y, 1, 0);

    // Button frame.
    int bx = ((dxWindow - dxButton) >> 1) - 1;
    DrawBorder(dc, bx, dyTopLed - 1, bx + dxButton + 1,
               dyTopLed + dyButton, 1, 2);
}

void Renderer::DrawScreen(wxDC& dc, const GameCore& game) {
    const int boardW = game.BoardWidth();
    const int boardH = game.BoardHeight();
    DrawBackground(dc, boardW, boardH);
    DrawBombCount(dc, game.BombsLeft());
    DrawButton(dc, boardW, game.ButtonState());
    DrawTime(dc, boardW, game.Seconds());
    DrawGrid(dc, game);
}

wxSize Renderer::ComputePanelSize(int boardW, int boardH) const {
    return wxSize(dxBlk * boardW + dxGridOff + dxRightSpace,
                  dyBlk * boardH + dyGridOff + dyBottomSpace);
}

} // namespace wxmine
