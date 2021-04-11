#include "assert.h"

#if _WIN32
#include <Windows.h>
#endif

namespace grabbed
{
    bool confirm_break(string& context)
    {
#if _WIN32
        auto result{ MessageBoxA(0, context.c_str(), "projectgrabbed", MB_ABORTRETRYIGNORE) };
        if (result == IDABORT) {
            abort();
        }
        return (result == IDRETRY);
#else
        return false;
#endif
    }

    bool hasDebugger()
    {
#if _WIN32
        return (IsDebuggerPresent() != FALSE);
#else
        return false;
#endif
    }
}
