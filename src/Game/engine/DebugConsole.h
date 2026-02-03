#ifndef _DEBUG_CONSOLE_H
#define _DEBUG_CONSOLE_H

#include <iostream>

// -- Debug assert --
// TODO: Move this to a more appropriate file...
#ifdef _DEBUG

#ifdef BUILD_PLATFORM_WINDOWS
#include <windows.h>

#define LB_ASSERT(condition, msg)                                               \
    do                                                                          \
    {                                                                           \
        if (!(condition))                                                       \
        {                                                                       \
            std::cout << "Error in '" << __FILE__ << "' at line #" << __LINE__; \
            std::cout << "\n  --> " << msg << "\n";                             \
            MessageBox(                                                         \
                NULL,                                                           \
                msg,                                                            \
                "Error",                                                        \
                MB_OK | MB_ICONERROR);                                          \
            __debugbreak();                                                     \
        }                                                                       \
    } while (0)
#else
// #define LB_ASSERT(condition, msg) do { if (!(condition)) { std::cout << "Error in '" << __FILE__ << "' at line #" << __LINE__; __debugbreak(); }} while (0)
#define LB_ASSERT(condition, msg)                                               \
    do                                                                          \
    {                                                                           \
        if (!(condition))                                                       \
        {                                                                       \
            std::cout << "Error in '" << __FILE__ << "' at line #" << __LINE__; \
            std::cout << "\n  --> " << msg << "\n";                             \
            __debugbreak();                                                     \
        }                                                                       \
    } while (0)
#endif

#else
#define LB_ASSERT(condition, msg)                   \
    do                                              \
    {                                               \
        if (!(condition))                           \
        {                                           \
            std::cout << "\n  --> " << msg << "\n"; \
        }                                           \
    } while (0)

#endif // _DEBUG

namespace Engine
{
    namespace DebugConsole
    {
        inline bool shown = false;

        // Create and initialize new console window
        void init();
        // Destroy debug console window
        void free();

        // Show the debug console window
        void show();
        // Hide the debug console window
        void hide();
    }
}

#endif // _DEBUG_CONSOLE_H