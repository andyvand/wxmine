#pragma once

// Game-wide constants shared by the core logic and the renderer.
// Derived from rtns.h / grafix.h / main.h in the original Win32 source.

namespace wxmine {

// Board size bounds (from main.h)
static constexpr int kMinWidth  = 9;
static constexpr int kMinHeight = 9;
static constexpr int kMaxWidth  = 30;
static constexpr int kMaxHeight = 24;

// Grid storage geometry — the original uses a 32-wide row stride so that
// IBLK(x,y) = rgBlk[(y<<5)+x]. We preserve that layout so the ported algorithm
// is bit-identical to the original flood-fill queue behavior.
static constexpr int kRowStride = 32;
static constexpr int kColStride = 27;
static constexpr int kBlkMax    = kColStride * kRowStride; // 864

// Block sprite indices (from rtns.h).
static constexpr int iBlkBlank    = 0;
static constexpr int iBlk1        = 1;
static constexpr int iBlk8        = 8;
static constexpr int iBlkGuessDn  = 9;
static constexpr int iBlkBombDn   = 10;
static constexpr int iBlkWrong    = 11;
static constexpr int iBlkExplode  = 12;
static constexpr int iBlkGuessUp  = 13;
static constexpr int iBlkBombUp   = 14;
static constexpr int iBlkBlankUp  = 15;
static constexpr int iBlkMax      = 16;

static constexpr unsigned char MaskBomb    = 0x80;
static constexpr unsigned char MaskVisit   = 0x40;
static constexpr unsigned char MaskFlags   = 0xE0;
static constexpr unsigned char MaskData    = 0x1F;
static constexpr unsigned char NotMaskBomb = 0x7F;

// LED sprite indices.
static constexpr int iLed0        = 0;
static constexpr int iLed9        = 9;
static constexpr int iLedBlank    = 10;
static constexpr int iLedNegative = 11;
static constexpr int iLedMax      = 12;

// Button sprite indices.
static constexpr int iButtonHappy   = 0;
static constexpr int iButtonCaution = 1;
static constexpr int iButtonLose    = 2;
static constexpr int iButtonWin     = 3;
static constexpr int iButtonDown    = 4;
static constexpr int iButtonMax     = 5;

// Game difficulty (from rtns.h: wGame*).
enum GameType {
    kGameBegin  = 0,
    kGameInter  = 1,
    kGameExpert = 2,
    kGameOther  = 3,
};

// Sprite dimensions (from grafix.h).
static constexpr int dxBlk    = 16;
static constexpr int dyBlk    = 16;
static constexpr int dxLed    = 13;
static constexpr int dyLed    = 23;
static constexpr int dxButton = 24;
static constexpr int dyButton = 24;

// Screen-margin geometry (from grafix.h).
static constexpr int dxLeftSpace   = 12;
static constexpr int dxRightSpace  = 12;
static constexpr int dyTopSpace    = 12;
static constexpr int dyBottomSpace = 12;

static constexpr int dxGridOff  = dxLeftSpace;
static constexpr int dyTopLed   = dyTopSpace + 4;
static constexpr int dyGridOff  = dyTopLed + dyLed + 16;

static constexpr int dxLeftBomb  = dxLeftSpace + 5;
static constexpr int dxRightTime = dxRightSpace + 5;

// Status bitfield (from rtns.h).
static constexpr int fPlay  = 0x01;
static constexpr int fPause = 0x02;
static constexpr int fPanic = 0x04;
static constexpr int fIcon  = 0x08;
static constexpr int fDemo  = 0x10;

// Win/lose outcome.
static constexpr bool kLose = false;
static constexpr bool kWin  = true;

// Sound tunes.
enum Tune {
    kTuneTick = 1,
    kTuneWin  = 2,
    kTuneLose = 3,
};

} // namespace wxmine
