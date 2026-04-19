// Windows CHM help launcher. Opens wxmine.chm in HTML Help Viewer using the
// HtmlHelp API (hhctrl.ocx). The CHM is shipped alongside wxmine.exe by the
// CMake POST_BUILD step, so we resolve it relative to the running executable
// rather than assuming a CWD.
//
// Mirrors help_mac.mm's ShowMacHelpBook(anchor) so callers can use one name
// per platform: ShowWinHelp(anchor) / ShowMacHelpBook(anchor).

#include <windows.h>
#include <htmlhelp.h>
#include <string>

extern "C" void ShowWinHelp(const char *anchor)
{
    wchar_t exePath[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (n == 0 || n == MAX_PATH) return;

    std::wstring chm(exePath, n);
    size_t slash = chm.find_last_of(L"\\/");
    chm.resize(slash == std::wstring::npos ? 0 : slash + 1);
    chm += L"wxmine.chm";

    if (anchor && *anchor) {
        // HtmlHelpW wants "path::/topic.htm" for HH_DISPLAY_TOPIC. Widen the
        // anchor from UTF-8; topic filenames in the CHM are ASCII so a plain
        // MultiByteToWideChar suffices.
        int wlen = MultiByteToWideChar(CP_UTF8, 0, anchor, -1, nullptr, 0);
        if (wlen > 0) {
            std::wstring wanchor(wlen - 1, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, anchor, -1, &wanchor[0], wlen);
            std::wstring target = chm + L"::/" + wanchor;
            HtmlHelpW(GetDesktopWindow(), target.c_str(), HH_DISPLAY_TOPIC, 0);
            return;
        }
    }

    HtmlHelpW(GetDesktopWindow(), chm.c_str(), HH_DISPLAY_TOPIC, 0);
}
