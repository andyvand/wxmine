#include "GameCore.h"

#include <algorithm>
#include <cstdlib>

#include "Preferences.h"
#include "SoundMgr.h"

namespace wxmine {

GameCore::GameCore(Preferences& prefs, SoundMgr& sound)
    : m_prefs(prefs), m_sound(sound) {}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void GameCore::Reset() {
    const bool resize = (m_prefs.Width != m_boardW) || (m_prefs.Height != m_boardH);

    m_timerRunning = false;
    m_boardW = m_prefs.Width;
    m_boardH = m_prefs.Height;

    ClearField();
    m_iButton = iButtonHappy;

    // Place mines (rtns.c::StartGame).
    int remaining = m_prefs.Mines;
    do {
        int x, y;
        do {
            x = (std::rand() % m_boardW) + 1;
            y = (std::rand() % m_boardH) + 1;
        } while (fISBOMB(x, y));
        SetBomb(x, y);
    } while (--remaining);

    m_cSec         = 0;
    m_cBombStart   = m_prefs.Mines;
    m_cBombLeft    = m_prefs.Mines;
    m_cBoxVisit    = 0;
    m_cBoxVisitMac = (m_boardW * m_boardH) - m_cBombLeft;
    m_status       = fPlay;

    UpdateBombCount(0);

    if (resize && m_resize) {
        m_resize(m_boardW, m_boardH);
    }
    if (m_repaintGrid)   m_repaintGrid();
    if (m_repaintButton) m_repaintButton();
    if (m_repaintTime)   m_repaintTime();
}

// ---------------------------------------------------------------------------
// Board manipulation (ported verbatim from rtns.c)
// ---------------------------------------------------------------------------

void GameCore::ClearField() {
    for (int i = 0; i < kBlkMax; ++i) {
        m_rgBlk[i] = static_cast<char>(iBlkBlankUp);
    }
    for (int i = 0; i < m_boardW + 2; ++i) {
        SetBorder(i, 0);
        SetBorder(i, m_boardH + 1);
    }
    for (int i = 0; i < m_boardH + 2; ++i) {
        SetBorder(0, i);
        SetBorder(m_boardW + 1, i);
    }
}

int GameCore::CountBombs(int xc, int yc) const {
    int n = 0;
    for (int y = yc - 1; y <= yc + 1; ++y)
        for (int x = xc - 1; x <= xc + 1; ++x)
            if (fISBOMB(x, y)) ++n;
    return n;
}

int GameCore::CountMarks(int xc, int yc) const {
    int n = 0;
    for (int y = yc - 1; y <= yc + 1; ++y)
        for (int x = xc - 1; x <= xc + 1; ++x)
            if (fGUESSBOMB(x, y)) ++n;
    return n;
}

void GameCore::ChangeBlk(int x, int y, int iBlk) {
    SetBlk(x, y, iBlk);
    RepaintCell(x, y);
}

void GameCore::ShowBombs(int iBlk) {
    for (int y = 1; y <= m_boardH; ++y) {
        for (int x = 1; x <= m_boardW; ++x) {
            if (!fVISIT(x, y)) {
                if (fISBOMB(x, y)) {
                    if (!fGUESSBOMB(x, y))
                        SetBlk(x, y, iBlk);
                } else if (fGUESSBOMB(x, y)) {
                    SetBlk(x, y, iBlkWrong);
                }
            }
        }
    }
    if (m_repaintGrid) m_repaintGrid();
}

void GameCore::GameOver(bool win) {
    m_timerRunning = false;
    m_iButton = win ? iButtonWin : iButtonLose;
    if (m_repaintButton) m_repaintButton();

    ShowBombs(win ? iBlkBombUp : iBlkBombDn);
    if (win && m_cBombLeft != 0) UpdateBombCount(-m_cBombLeft);
    m_sound.Play(win ? kTuneWin : kTuneLose, m_prefs);
    m_status = fDemo;

    if (win
        && m_prefs.wGameType != kGameOther
        && m_cSec < m_prefs.rgTime[m_prefs.wGameType])
    {
        m_prefs.rgTime[m_prefs.wGameType] = m_cSec;
        if (m_newBest)     m_newBest(m_prefs.wGameType);
        if (m_displayBest) m_displayBest();
    }
}

void GameCore::StepXY(int x, int y) {
    int idx = (y << 5) + x;
    int blk = static_cast<unsigned char>(m_rgBlk[idx]);

    if ((blk & MaskVisit)
        || ((blk & MaskData) == iBlkMax)
        || ((blk & MaskData) == iBlkBombUp))
        return;

    ++m_cBoxVisit;
    int cBombs = CountBombs(x, y);
    m_rgBlk[idx] = static_cast<char>(MaskVisit | cBombs);
    RepaintCell(x, y);

    if (cBombs != 0) return;

    m_rgStepX[m_iStepMac] = x;
    m_rgStepY[m_iStepMac] = y;
    if (++m_iStepMac == kStepMax) m_iStepMac = 0;
}

void GameCore::StepBox(int x, int y) {
    int iCur = 0;
    m_iStepMac = 1;

    StepXY(x, y);
    if (++iCur != m_iStepMac) {
        while (iCur != m_iStepMac) {
            x = m_rgStepX[iCur];
            y = m_rgStepY[iCur];

            StepXY(x - 1, y - 1);
            StepXY(x,     y - 1);
            StepXY(x + 1, y - 1);

            StepXY(x - 1, y);
            StepXY(x + 1, y);

            StepXY(x - 1, y + 1);
            StepXY(x,     y + 1);
            StepXY(x + 1, y + 1);

            if (++iCur == kStepMax) iCur = 0;
        }
    }
}

void GameCore::StepSquare(int x, int y) {
    if (fISBOMB(x, y)) {
        if (m_cBoxVisit == 0) {
            // First click is always safe — relocate the bomb.
            for (int yt = 1; yt < m_boardH; ++yt) {
                for (int xt = 1; xt < m_boardW; ++xt) {
                    if (!fISBOMB(xt, yt)) {
                        IBLK_REF(x, y) = static_cast<char>(iBlkBlankUp);
                        SetBomb(xt, yt);
                        StepBox(x, y);
                        return;
                    }
                }
            }
        } else {
            ChangeBlk(x, y, MaskVisit | iBlkExplode);
            GameOver(kLose);
        }
    } else {
        StepBox(x, y);
        if (CheckWin()) GameOver(kWin);
    }
}

void GameCore::StepBlock(int xc, int yc) {
    if (!fVISIT(xc, yc) || iBLK(xc, yc) != CountMarks(xc, yc)) {
        TrackMouse(-2, -2); // pop the cells back up
        return;
    }
    bool over = false;
    for (int y = yc - 1; y <= yc + 1; ++y) {
        for (int x = xc - 1; x <= xc + 1; ++x) {
            if (!fGUESSBOMB(x, y) && fISBOMB(x, y)) {
                over = true;
                ChangeBlk(x, y, MaskVisit | iBlkExplode);
            } else {
                StepBox(x, y);
            }
        }
    }
    if (over)        GameOver(kLose);
    else if (CheckWin()) GameOver(kWin);
}

void GameCore::PushBoxDown(int x, int y) {
    int b = iBLK(x, y);
    if (b == iBlkGuessUp)      b = iBlkGuessDn;
    else if (b == iBlkBlankUp) b = iBlkBlank;
    SetBlk(x, y, b);
}

void GameCore::PopBoxUp(int x, int y) {
    int b = iBLK(x, y);
    if (b == iBlkGuessDn)      b = iBlkGuessUp;
    else if (b == iBlkBlank)   b = iBlkBlankUp;
    SetBlk(x, y, b);
}

void GameCore::TrackMouse(int xn, int yn) {
    if (xn == m_xCur && yn == m_yCur) return;

    int xOld = m_xCur, yOld = m_yCur;
    m_xCur = xn;
    m_yCur = yn;

    if (m_blockMode) {
        bool validNew = InRange(xn, yn);
        bool validOld = InRange(xOld, yOld);

        int yOldMin = std::max(yOld - 1, 1);
        int yOldMax = std::min(yOld + 1, m_boardH);
        int yCurMin = std::max(m_yCur - 1, 1);
        int yCurMax = std::min(m_yCur + 1, m_boardH);
        int xOldMin = std::max(xOld - 1, 1);
        int xOldMax = std::min(xOld + 1, m_boardW);
        int xCurMin = std::max(m_xCur - 1, 1);
        int xCurMax = std::min(m_xCur + 1, m_boardW);

        if (validOld) {
            for (int y = yOldMin; y <= yOldMax; ++y)
                for (int x = xOldMin; x <= xOldMax; ++x)
                    if (!fVISIT(x, y)) PopBoxUp(x, y);
        }
        if (validNew) {
            for (int y = yCurMin; y <= yCurMax; ++y)
                for (int x = xCurMin; x <= xCurMax; ++x)
                    if (!fVISIT(x, y)) PushBoxDown(x, y);
        }
        if (validOld) {
            for (int y = yOldMin; y <= yOldMax; ++y)
                for (int x = xOldMin; x <= xOldMax; ++x)
                    RepaintCell(x, y);
        }
        if (validNew) {
            for (int y = yCurMin; y <= yCurMax; ++y)
                for (int x = xCurMin; x <= xCurMax; ++x)
                    RepaintCell(x, y);
        }
    } else {
        if (InRange(xOld, yOld) && !fVISIT(xOld, yOld)) {
            PopBoxUp(xOld, yOld);
            RepaintCell(xOld, yOld);
        }
        if (InRange(xn, yn) && ValidStep(xn, yn)) {
            PushBoxDown(m_xCur, m_yCur);
            RepaintCell(m_xCur, m_yCur);
        }
    }
}

void GameCore::MakeGuess(int x, int y) {
    if (!InRange(x, y) || fVISIT(x, y)) return;

    int b;
    if (fGUESSBOMB(x, y)) {
        b = m_prefs.fMark ? iBlkGuessUp : iBlkBlankUp;
        UpdateBombCount(+1);
    } else if (fGUESSMARK(x, y)) {
        b = iBlkBlankUp;
    } else {
        b = iBlkBombUp;
        UpdateBombCount(-1);
    }
    ChangeBlk(x, y, b);

    if (fGUESSBOMB(x, y) && CheckWin()) GameOver(kWin);
}

void GameCore::UpdateBombCount(int delta) {
    m_cBombLeft += delta;
    if (m_repaintCount) m_repaintCount();
}

// ---------------------------------------------------------------------------
// Mouse-event entry points called by BoardPanel.
// ---------------------------------------------------------------------------

void GameCore::OnLeftDown(int boardX, int boardY, bool chord) {
    if (!(m_status & fPlay)) return;
    m_blockMode = chord;
    m_button1Down = true;
    m_xCur = -1;
    m_yCur = -1;
    m_iButton = iButtonCaution;
    if (m_repaintButton) m_repaintButton();
    TrackMouse(boardX, boardY);
}

void GameCore::OnRightDown(int boardX, int boardY) {
    if (!(m_status & fPlay)) return;
    if (m_button1Down) {
        // Left+right chord behavior — switch to block-mode tracking.
        TrackMouse(-3, -3);
        m_blockMode = true;
        TrackMouse(boardX, boardY);
    } else {
        MakeGuess(boardX, boardY);
    }
}

void GameCore::OnMiddleDown(int boardX, int boardY) {
    if (!(m_status & fPlay)) return;
    m_blockMode = true;
    m_button1Down = true;
    m_xCur = -1;
    m_yCur = -1;
    m_iButton = iButtonCaution;
    if (m_repaintButton) m_repaintButton();
    TrackMouse(boardX, boardY);
}

void GameCore::OnButton1Up() {
    if (!m_button1Down) return;
    m_button1Down = false;

    if (InRange(m_xCur, m_yCur)) {
        if (m_cBoxVisit == 0 && m_cSec == 0) {
            m_sound.Play(kTuneTick, m_prefs);
            ++m_cSec;
            if (m_repaintTime) m_repaintTime();
            m_timerRunning = true;
        }
        if (!(m_status & fPlay)) m_xCur = m_yCur = -2;

        if (m_blockMode)        StepBlock(m_xCur, m_yCur);
        else if (ValidStep(m_xCur, m_yCur)) StepSquare(m_xCur, m_yCur);
    }

    if (m_status & fPlay) m_iButton = iButtonHappy;
    if (m_repaintButton) m_repaintButton();
}

// ---------------------------------------------------------------------------
// Pause/resume + timer
// ---------------------------------------------------------------------------

void GameCore::Pause() {
    m_sound.Stop();
    if (!(m_status & fPause)) m_oldTimerStatus = m_timerRunning;
    if (m_status & fPlay) m_timerRunning = false;
    m_status |= fPause;
}

void GameCore::Resume() {
    if (m_status & fPlay) m_timerRunning = m_oldTimerStatus;
    m_status &= ~fPause;
}

void GameCore::Tick() {
    if (m_timerRunning && m_cSec < 999) {
        ++m_cSec;
        if (m_repaintTime) m_repaintTime();
        m_sound.Play(kTuneTick, m_prefs);
    }
}

} // namespace wxmine
