#include "MainFrame.h"

#include <memory>

#include <wx/aboutdlg.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/xrc/xmlres.h>

#include "BestDialog.h"
#include "BoardPanel.h"
#include "EnterDialog.h"
#include "GameCore.h"
#include "GameConstants.h"
#include "PrefDialog.h"
#include "Renderer.h"

namespace wxmine {

// Owns the heap objects that need a destructor sequence after MainFrame's
// children are torn down. (Renderer and GameCore are not wxWindow children,
// so they need explicit ownership.)
struct MainFrame::Owned {
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<GameCore> game;
};

namespace {

// Difficulty presets — same numbers as the original winmine.c rgLevelData.
struct LevelData { int mines, height, width; };
constexpr LevelData kLevels[3] = {
    { 10,  9,  9 },   // Beginner
    { 40, 16, 16 },   // Intermediate
    { 99, 16, 30 },   // Expert
};

} // namespace

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Minesweeper",
              wxDefaultPosition, wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
    m_owned = new Owned;

    m_prefs.Load();

    // Menu bar from XRC.
    SetMenuBar(wxXmlResource::Get()->LoadMenuBar("ID_MENU"));

    // Window icon: defined in winmine.xrc as <object class="wxIcon"
    // name="ID_APP_ICON">assets/wxmine.png</object>, so wxXmlResource
    // resolves the PNG out of the embedded .xrs archive on our behalf.
    const wxIcon icon = wxXmlResource::Get()->LoadIcon("ID_APP_ICON");
    if (icon.IsOk()) {
        SetIcon(icon);
    }

    // Renderer + game core.
    m_owned->renderer = std::make_unique<Renderer>();
    if (!m_owned->renderer->Reload(m_prefs.fColor)) {
        wxMessageBox("Failed to load embedded sprite assets.", "Minesweeper",
                     wxICON_ERROR | wxOK, this);
    }
    m_renderer = m_owned->renderer.get();

    // Sound.
    if (m_prefs.fSound == 3) {
        m_prefs.fSound = m_sound.Init();
    } else {
        m_sound.Init(); // Pre-load anyway so toggling on works.
    }

    m_owned->game = std::make_unique<GameCore>(m_prefs, m_sound);
    m_game = m_owned->game.get();

    // Board panel as the only child.
    m_panel = new BoardPanel(this, *m_game, *m_renderer);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_panel, 1, wxEXPAND);
    SetSizerAndFit(sizer);

    // Wire callbacks from GameCore back to the panel and frame.
    m_game->SetRepaintCell  ([this](int x, int y){ m_panel->RepaintCell(x, y); });
    m_game->SetRepaintGrid  ([this]{ m_panel->RepaintGrid(); });
    m_game->SetRepaintButton([this]{ m_panel->RepaintButton(); });
    m_game->SetRepaintCount ([this]{ m_panel->RepaintBombCount(); });
    m_game->SetRepaintTime  ([this]{ m_panel->RepaintTime(); });
    m_game->SetResize       ([this](int w, int h){ ResizeForBoard(w, h); });
    m_game->SetNewBest      ([this](int level){
        EnterDialog dlg(this, m_prefs, level);
        dlg.ShowModal();
    });
    m_game->SetDisplayBest  ([this]{
        BestDialog dlg(this, m_prefs);
        dlg.ShowModal();
    });

    // Bind menu events. Using XRCID lets the XRC and the C++ stay in sync.
    Bind(wxEVT_MENU, &MainFrame::OnNew,           this, XRCID("IDM_NEW"));
    Bind(wxEVT_MENU, &MainFrame::OnSetDifficulty, this, XRCID("IDM_BEGIN"));
    Bind(wxEVT_MENU, &MainFrame::OnSetDifficulty, this, XRCID("IDM_INTER"));
    Bind(wxEVT_MENU, &MainFrame::OnSetDifficulty, this, XRCID("IDM_EXPERT"));
    Bind(wxEVT_MENU, &MainFrame::OnCustom,        this, XRCID("IDM_CUSTOM"));
    Bind(wxEVT_MENU, &MainFrame::OnToggleMark,    this, XRCID("IDM_MARK"));
    Bind(wxEVT_MENU, &MainFrame::OnToggleColor,   this, XRCID("IDM_COLOR"));
    Bind(wxEVT_MENU, &MainFrame::OnToggleSound,   this, XRCID("IDM_SOUND"));
    Bind(wxEVT_MENU, &MainFrame::OnBest,          this, XRCID("IDM_BEST"));
    Bind(wxEVT_MENU, &MainFrame::OnExit,          this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::OnAbout,         this, wxID_ABOUT);

    Bind(wxEVT_ICONIZE, &MainFrame::OnIconize, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

    // F2 → New Game accelerator (the menu label "&New\tF2" handles this on
    // most platforms automatically, but be explicit for portability).
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_F2, XRCID("IDM_NEW"));
    SetAcceleratorTable(wxAcceleratorTable(1, entries));

    // Initial board.
    m_game->Reset();
    FixMenus();

    Move(m_prefs.xWindow, m_prefs.yWindow);
}

MainFrame::~MainFrame() {
    delete m_owned;
}

void MainFrame::ResizeForBoard(int boardW, int boardH) {
    const wxSize sz = m_panel->ResizeForBoard(boardW, boardH);
    SetClientSize(sz);
    Layout();
}

void MainFrame::FixMenus() {
    auto check = [&](const char* name, bool on) {
        wxMenuBar* mb = GetMenuBar();
        if (auto* item = mb->FindItem(XRCID(name))) item->Check(on);
    };
    check("IDM_BEGIN",  m_prefs.wGameType == kGameBegin);
    check("IDM_INTER",  m_prefs.wGameType == kGameInter);
    check("IDM_EXPERT", m_prefs.wGameType == kGameExpert);
    check("IDM_CUSTOM", m_prefs.wGameType == kGameOther);
    check("IDM_MARK",   m_prefs.fMark);
    check("IDM_COLOR",  m_prefs.fColor);
    check("IDM_SOUND",  m_prefs.fSound == 3);
}

void MainFrame::OnNew(wxCommandEvent&) {
    m_game->Reset();
}

void MainFrame::OnSetDifficulty(wxCommandEvent& evt) {
    int level = kGameBegin;
    if (evt.GetId() == XRCID("IDM_INTER"))  level = kGameInter;
    if (evt.GetId() == XRCID("IDM_EXPERT")) level = kGameExpert;

    m_prefs.wGameType = level;
    m_prefs.Mines  = kLevels[level].mines;
    m_prefs.Height = kLevels[level].height;
    m_prefs.Width  = kLevels[level].width;
    FixMenus();
    m_game->Reset();
}

void MainFrame::OnCustom(wxCommandEvent&) {
    PrefDialog dlg(this, m_prefs);
    if (dlg.ShowModal() == wxID_OK) {
        m_prefs.wGameType = kGameOther;
        FixMenus();
        m_game->Reset();
    }
}

void MainFrame::OnToggleMark(wxCommandEvent&) {
    m_prefs.fMark = !m_prefs.fMark;
    FixMenus();
}

void MainFrame::OnToggleColor(wxCommandEvent&) {
    m_prefs.fColor = !m_prefs.fColor;
    if (!m_renderer->Reload(m_prefs.fColor)) {
        // Fall back if the B&W asset is missing.
        m_prefs.fColor = true;
        m_renderer->Reload(true);
    }
    FixMenus();
    m_panel->Refresh();
}

void MainFrame::OnToggleSound(wxCommandEvent&) {
    if (m_prefs.fSound == 3) {
        m_sound.Stop();
        m_prefs.fSound = 2;
    } else {
        m_prefs.fSound = m_sound.Init();
    }
    FixMenus();
}

void MainFrame::OnBest(wxCommandEvent&) {
    BestDialog dlg(this, m_prefs);
    dlg.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent&) {
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent&) {
    wxAboutDialogInfo info;
    info.SetName("Minesweeper");
    info.SetVersion("1.0 (wxWidgets port)");
    info.SetDescription("Classic Minesweeper, ported to portable wxWidgets.");
    info.SetCopyright("Original game (c) Microsoft.\n"
                      "by Robert Donner and Curt Johnson");
    info.SetIcon(GetIcon());
    wxAboutBox(info, this);
}

void MainFrame::OnIconize(wxIconizeEvent& evt) {
    if (evt.IsIconized()) m_game->Pause();
    else                  m_game->Resume();
    evt.Skip();
}

void MainFrame::OnClose(wxCloseEvent& evt) {
    const wxPoint pos = GetPosition();
    m_prefs.xWindow = pos.x;
    m_prefs.yWindow = pos.y;
    m_prefs.Save();
    evt.Skip();
}

} // namespace wxmine
