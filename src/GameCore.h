#pragma once

#include <array>
#include <functional>

#include "GameConstants.h"

namespace wxmine {

struct Preferences;
class SoundMgr;

// Ported from rtns.c. The board state machine — bomb placement, flood-fill,
// flagging/marking, win/lose detection — is preserved bit-for-bit. UI side
// effects that the original called inline (DisplayBlk, DisplayButton,
// DisplayTime, DisplayBombCount, AdjustWindow) become callbacks the host
// (BoardPanel/MainFrame) installs at construction.
class GameCore {
public:
    // Per-cell repaint hint. (-1, -1) means "repaint the whole board".
    using RepaintCellFn  = std::function<void(int x, int y)>;
    using RepaintGridFn  = std::function<void()>;
    using RepaintButtonFn= std::function<void()>;
    using RepaintCountFn = std::function<void()>;
    using RepaintTimeFn  = std::function<void()>;
    using ResizeFn       = std::function<void(int width, int height)>;
    using NewBestFn      = std::function<void(int level)>;
    using DisplayBestFn  = std::function<void()>;

    GameCore(Preferences& prefs, SoundMgr& sound);

    // ---- Lifecycle ----------------------------------------------------------

    void Reset();                 // Equivalent of StartGame() in rtns.c.

    // ---- Mouse-driven actions (called by BoardPanel) -----------------------

    // Down events update internal state but don't reveal anything yet.
    void OnLeftDown(int boardX, int boardY, bool chord);
    void OnRightDown(int boardX, int boardY);
    void OnMiddleDown(int boardX, int boardY);

    // Up event: chooses StepSquare/StepBlock based on the chord flag set on
    // the matching down event.
    void OnButton1Up();

    // Mouse moved while a button is held — animates the pushed-down preview.
    void TrackMouse(int boardX, int boardY);

    // ---- Pause/resume (used by MainFrame on iconify/restore) ---------------

    void Pause();
    void Resume();

    // ---- Timer tick (1 Hz) -------------------------------------------------

    void Tick();

    // ---- Queries used by Renderer ------------------------------------------

    int  BoardWidth()  const { return m_boardW; }
    int  BoardHeight() const { return m_boardH; }
    int  BombsLeft()   const { return m_cBombLeft; }
    int  Seconds()     const { return m_cSec; }
    int  ButtonState() const { return m_iButton; }

    // Returns the visible block index (iBlk*) for a 1-based grid cell.
    // Strips the bomb/visit flag bits so the renderer can index its sprite
    // sheet directly.
    int  VisibleBlk(int x, int y) const {
        return static_cast<unsigned char>(m_rgBlk[(y << 5) + x]) & MaskData;
    }

    // True iff the player has the cursor pressed down on the smiley (used by
    // MainFrame to draw the iButtonDown sprite while the smiley is held).
    bool IsBoardPlayable() const { return (m_status & fPlay) != 0; }

    // ---- Repaint/resize callbacks installed by the host --------------------

    void SetRepaintCell  (RepaintCellFn  fn) { m_repaintCell   = std::move(fn); }
    void SetRepaintGrid  (RepaintGridFn  fn) { m_repaintGrid   = std::move(fn); }
    void SetRepaintButton(RepaintButtonFn fn){ m_repaintButton = std::move(fn); }
    void SetRepaintCount (RepaintCountFn fn) { m_repaintCount  = std::move(fn); }
    void SetRepaintTime  (RepaintTimeFn  fn) { m_repaintTime   = std::move(fn); }
    void SetResize       (ResizeFn       fn) { m_resize        = std::move(fn); }
    void SetNewBest      (NewBestFn      fn) { m_newBest       = std::move(fn); }
    void SetDisplayBest  (DisplayBestFn  fn) { m_displayBest   = std::move(fn); }

    // After the host plays the StartGame countdown / first-tick logic and
    // wants to start the seconds timer, IsTimerRunning lets it tell whether
    // it should arm its wxTimer.
    bool IsTimerRunning() const { return m_timerRunning; }

    // Visual-only smiley state for "user is currently pressing a cell".
    void SetButtonCaution() { m_iButton = iButtonCaution; if (m_repaintButton) m_repaintButton(); }
    void SetButtonHappy()   { m_iButton = iButtonHappy;   if (m_repaintButton) m_repaintButton(); }

private:
    // ---- State (mirrors rtns.c globals) ------------------------------------

    Preferences& m_prefs;
    SoundMgr&    m_sound;

    // Initialised to 0 (not the default board size) so the very first Reset()
    // always sees a dimension change and fires the resize callback. Otherwise
    // a fresh launch with default 9x9 prefs would skip the resize and the
    // top-level frame would stay at its pre-fit client size of (0, 0).
    int   m_boardW       = 0;
    int   m_boardH       = 0;
    int   m_cBombStart   = 10;
    int   m_cBombLeft    = 10;
    int   m_cBoxVisit    = 0;
    int   m_cBoxVisitMac = 0;
    int   m_cSec         = 0;
    int   m_xCur         = -1;
    int   m_yCur         = -1;
    int   m_iButton      = iButtonHappy;
    int   m_status       = fDemo | fIcon;
    bool  m_blockMode    = false;          // chord (left+right or middle)
    bool  m_button1Down  = false;
    bool  m_timerRunning = false;
    bool  m_oldTimerStatus = false;

    // 27×32 = 864 byte board, with row stride 32 to match rtns.c's IBLK macro.
    std::array<char, kBlkMax> m_rgBlk{};

    // Flood-fill queue (rtns.c rgStepX/Y).
    static constexpr int kStepMax = 100;
    int m_rgStepX[kStepMax] = {};
    int m_rgStepY[kStepMax] = {};
    int m_iStepMac = 0;

    // Repaint callbacks installed by the host.
    RepaintCellFn   m_repaintCell;
    RepaintGridFn   m_repaintGrid;
    RepaintButtonFn m_repaintButton;
    RepaintCountFn  m_repaintCount;
    RepaintTimeFn   m_repaintTime;
    ResizeFn        m_resize;
    NewBestFn       m_newBest;
    DisplayBestFn   m_displayBest;

    // ---- Block accessors (replace the C macros in rtns.h) -----------------

    inline int   IBLK(int x, int y) const { return static_cast<unsigned char>(m_rgBlk[(y << 5) + x]); }
    inline char& IBLK_REF(int x, int y)   { return m_rgBlk[(y << 5) + x]; }
    inline int   iBLK(int x, int y) const { return IBLK(x, y) & MaskData; }
    inline bool  fISBOMB(int x, int y) const { return (IBLK(x, y) & MaskBomb)  != 0; }
    inline bool  fVISIT(int x, int y)  const { return (IBLK(x, y) & MaskVisit) != 0; }
    inline bool  fGUESSBOMB(int x, int y) const { return iBLK(x, y) == iBlkBombUp; }
    inline bool  fGUESSMARK(int x, int y) const { return iBLK(x, y) == iBlkGuessUp; }
    inline bool  fBORDER(int x, int y) const { return IBLK(x, y) == iBlkMax; }
    inline bool  InRange(int x, int y) const {
        return x > 0 && y > 0 && x <= m_boardW && y <= m_boardH;
    }
    inline bool  ValidStep(int x, int y) const {
        return !(fVISIT(x, y) || fGUESSBOMB(x, y));
    }

    inline void  SetBomb(int x, int y)   { IBLK_REF(x, y) |= MaskBomb; }
    inline void  SetBorder(int x, int y) { IBLK_REF(x, y) = (char)iBlkMax; }
    inline void  SetVisit(int x, int y)  { IBLK_REF(x, y) |= MaskVisit; }
    inline void  SetBlk(int x, int y, int blk) {
        IBLK_REF(x, y) = static_cast<char>((IBLK(x, y) & MaskFlags) | blk);
    }

    // ---- Internal algorithms (ported from rtns.c) -------------------------

    void ChangeBlk(int x, int y, int iBlk);
    void ClearField();
    int  CountBombs(int xc, int yc) const;
    int  CountMarks(int xc, int yc) const;
    void ShowBombs(int iBlk);
    void GameOver(bool win);
    void StepXY(int x, int y);
    void StepBox(int x, int y);
    void StepSquare(int x, int y);
    void StepBlock(int xc, int yc);
    void PushBoxDown(int x, int y);
    void PopBoxUp(int x, int y);
    void MakeGuess(int x, int y);
    void UpdateBombCount(int delta);

    bool CheckWin() const { return m_cBoxVisit == m_cBoxVisitMac; }

    // Convenience: flag/unflag actually needs ChangeBlk so DisplayBlk fires.
    void RepaintCell(int x, int y) {
        if (m_repaintCell) m_repaintCell(x, y);
    }
};

} // namespace wxmine
