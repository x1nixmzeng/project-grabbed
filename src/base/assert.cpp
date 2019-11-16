#include "assert.h"

#include "base/stringify.h"

#if _WIN32
#include <Windows.h>
#endif

namespace grabbed
{
    namespace
    {
        bool confirm_break(const string& context)
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
    }

    bool confirm(const char* title, const char* function, unsigned int line, const char* context)
    {
        base::stringify str;
        
        str.append(title).append(" failed").eol();
        str.append("Method: ").append(function).eol();
        str.append("Line: ").append(line).eol();
        str.append(context);

        return confirm_break(str());
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
