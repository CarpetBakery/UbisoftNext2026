#include "DebugConsole.h"

#if BUILD_PLATFORM_WINDOWS
#include <main.h>

// Console window handle
namespace
{
    HWND hwnd;
}
#endif // BUILD_PLATFORM_WINDOWS

using namespace Engine;

void DebugConsole::init()
{
#if BUILD_PLATFORM_WINDOWS
    hwnd = GetConsoleWindow();
    if (hwnd == NULL)
    {
        // -- Create new console window --
        AllocConsole();

        // Assert that our streams are valid
        // (see: https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output)
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if ((hStdout == INVALID_HANDLE_VALUE) ||
            (hStdin == INVALID_HANDLE_VALUE))
        {
            ExitProcess(1);
        }

        // -- Redirect std out/in/err --
        FILE *fpOut, *fpIn, *fpErr;
        freopen_s(&fpOut, "CONOUT$", "w", stdout);
        freopen_s(&fpErr, "CONOUT$", "w", stderr);
        freopen_s(&fpIn, "CONIN$", "r", stdin);

        // Sync C++ streams with C
        std::ios::sync_with_stdio();
        std::cout.clear();
        std::cerr.clear();
        std::cin.clear();
    }

    // Put console window behind the main window
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindow
    hwnd = GetConsoleWindow();

    // Center main window
    SetWindowPos(MAIN_WINDOW_HANDLE, HWND_TOP, 1920 / 2 - WINDOW_WIDTH / 2, 1080 / 2 - WINDOW_HEIGHT / 2, 0, 0, SWP_NOSIZE);

    // Focus main window 
    SetActiveWindow(MAIN_WINDOW_HANDLE);
    SetForegroundWindow(MAIN_WINDOW_HANDLE);
    SetFocus(MAIN_WINDOW_HANDLE);
#endif // BUILD_PLATFORM_WINDOWS
}

void DebugConsole::free()
{
#if BUILD_PLATFORM_WINDOWS
    FreeConsole();
    hwnd = NULL;
#endif // BUILD_PLATFORM_WINDOWS
}

void DebugConsole::show()
{
#if BUILD_PLATFORM_WINDOWS
    // ShowWindow(consoleHandle, SW_SHOWMAXIMIZED);
    ShowWindow(hwnd, SW_SHOW);
#endif // BUILD_PLATFORM_WINDOWS
}

void DebugConsole::hide()
{
#if BUILD_PLATFORM_WINDOWS
    ShowWindow(hwnd, SW_HIDE);
#endif // BUILD_PLATFORM_WINDOWS
}
