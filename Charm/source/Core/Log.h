#pragma once
#include "Core/Base.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define INFO(message, ...) Charm::Core::Log("Info:", message, Charm::Core::TextColor::Green, ##__VA_ARGS__)
#define WARN(message, ...) Charm::Core::Log("Warn:", message, Charm::Core::TextColor::Yellow, ##__VA_ARGS__)
#define ERROR(message, ...) Charm::Core::Log("Error:", message, Charm::Core::TextColor::LightRed, ##__VA_ARGS__)
#define FATAL(message, ...) Charm::Core::Log("Fatal:", message, Charm::Core::TextColor::Red, ##__VA_ARGS__)
#define ASSERT(expression, message, ...)   \
    {                                      \
        if (!(expression))                 \
        {                                  \
            FATAL(message, ##__VA_ARGS__); \
            exit(420);                     \
        }                                  \
    }

namespace Charm
{
    namespace Core
    {
        enum class TextColor
        {
            Black = 0,
            Red,
            Green,
            Yellow,
            Blue,
            Magenta,
            Cyan,
            White,
            LightBlack,
            LightRed,
            LightGreen,
            LightYellow,
            LightBlue,
            LightMagenta,
            LightCyan,
            LightWhite,
            Count,
        };

        template <typename... Args>
        inline void Log(const char* prefix, const char* message, TextColor color, Args... args)
        {
            static const char* colorTable[(u32)TextColor::Count] = {
                "\x1b[30m", // Black
                "\x1b[31m", // Red
                "\x1b[32m", // Green
                "\x1b[33m", // Yellow
                "\x1b[34m", // Blue
                "\x1b[35m", // Magenta
                "\x1b[36m", // Cyan
                "\x1b[37m", // White
                "\x1b[90m", // Light black
                "\x1b[91m", // Light red
                "\x1b[92m", // Light green
                "\x1b[93m", // Light yellow
                "\x1b[94m", // Light blue
                "\x1b[95m", // Light magenta
                "\x1b[96m", // Light cyan
                "\x1b[97m", // Light white
            };

            char formatBuffer[8192]{};
            sprintf(formatBuffer, "%s %s %s \033[0m", colorTable[(u32)color], prefix, message);

            char textBuffer[8192]{};
            sprintf(textBuffer, formatBuffer, args...);

            puts(textBuffer);
        }
    }
}
