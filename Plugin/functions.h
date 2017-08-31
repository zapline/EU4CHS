#pragma once
#include "stdinc.h"
#include "eu4.h"

namespace Functions
{
    void __fastcall ConvertUTF8ToLatin1(const char *source, char *dest);
    void ConvertLatin1ToUTF8(const char *source, char *dest);
    bool IsNativeChar(uint32_t cp);
    bool IsTextIconChar(uint32_t cp);
    bool IsSpecialChar(uint32_t cp);

    void InitAndPatch();
};
