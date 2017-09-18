﻿#include "stdinc.h"
#include "functions.h"
#include "eu4.h"
#include "byte_pattern.h"

using namespace std;

namespace Functions
{
    void __fastcall ConvertUTF8ToLatin1(const char *source, char *dest)
    {
        string_view source_view(source);

        vector<wchar_t> wideText;
        utf8::utf8to16(source_view.begin(), source_view.end(), back_inserter(wideText));

        for (wchar_t &cp : wideText)
        {
            switch (cp)
            {
            case 0x80:
            case 0x82:
            case 0x83:
            case 0x84:
            case 0x85:
            case 0x86:
            case 0x87:
            case 0x88:
            case 0x89:
            case 0x8a:
            case 0x8b:
            case 0x8c:
            case 0x8e:
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:
            case 0x96:
            case 0x97:
            case 0x98:
            case 0x99:
            case 0x9a:
            case 0x9b:
            case 0x9c:
            case 0x9e:
            case 0x9f:
                cp = '?';
                break;

            case 0xA3:
            case 0xA4:
            case 0xA7:
                cp -= 0xA0;
                break;

            case 0x152: cp = 0x8c; break;
            case 0x153: cp = 0x9c; break;
            case 0x160: cp = 0x8a; break;
            case 0x161: cp = 0x9a; break;
            case 0x178: cp = 0x9f; break;
            case 0x17d: cp = 0x8e; break;
            case 0x17e: cp = 0x9e; break;
            case 0x192: cp = 0x83; break;
            case 0x2c6: cp = 0x88; break;
            case 0x2dc: cp = 0x98; break;
            case 0x2013: cp = 0x96; break;
            case 0x2014: cp = 0x97; break;
            case 0x2018: cp = 0x91; break;
            case 0x2019: cp = 0x92; break;
            case 0x201a: cp = 0x82; break;
            case 0x201c: cp = 0x93; break;
            case 0x201d: cp = 0x94; break;
            case 0x201e: cp = 0x84; break;
            case 0x2020: cp = 0x86; break;
            case 0x2021: cp = 0x87; break;
            case 0x2022: cp = 0x95; break;
            case 0x2026: cp = 0x85; break;
            case 0x2030: cp = 0x89; break;
            case 0x2039: cp = 0x8b; break;
            case 0x203a: cp = 0x9b; break;
            case 0x20ac: cp = 0x80; break;
            case 0x2122: cp = 0x99; break;

            default:
                break;
            }
        }

        wideText.push_back(0);
        utf8::utf16to8(wideText.begin(), wideText.end(), dest);
    }

    bool IsLatin1Char(uint32_t cp)
    {
        return cp <= 0xFF;
    }

    bool IsTextIconChar(uint32_t cp)
    {
        return isalpha(cp) || isdigit(cp) || cp == '_' || cp == '|';
    }

    void GetTwoUnicode(const char * pText, uint32_t index, uint32_t text_length, uint32_t &first, ptrdiff_t &first_length, uint32_t &second, bool bUseSpecialChars)
    {
        second = 0;

        try
        {
            //First
            if (index < text_length)
            {
                first_length = utf8::internal::sequence_length(pText);
                first = utf8::peek_next(pText + index, pText + text_length);
                index += first_length;
            }

            //Second
            if (index < text_length)
            {
                ptrdiff_t second_length = utf8::internal::sequence_length(pText + index);
                uint32_t temp_second = utf8::peek_next(pText + index, pText + text_length);

                if (bUseSpecialChars)
                {
                    while (temp_second == 0x7)
                    {
                        index += 2;

                        if (index >= text_length)
                        {
                            return;
                        }

                        temp_second = utf8::peek_next(pText + index, pText + text_length);
                    }
                }

                second = temp_second;
            }
        }
        catch (const utf8::invalid_utf8&)
        {
            wchar_t text[4096];

            MultiByteToWideChar(28591, 0, pText, text_length, text, 4096);
            MessageBoxW(NULL, text, L"Fucked text", MB_OK);
        }
    }

    void GetTwoUnicode(std::vector<wchar_t>::iterator pText, std::vector<wchar_t>::iterator pEnd, uint32_t & first, uint32_t & second, bool bUseSpecialChars)
    {
        second = 0;

        //First
        if (pText < pEnd)
        {
            first = *pText;
            ++pText;
        }

        //Second
        if (pText < pEnd)
        {
            uint32_t temp_second = *pText;

            if (bUseSpecialChars)
            {
                while (temp_second == 0x7)
                {
                    pText += 2;

                    if (pText >= pEnd)
                    {
                        return;
                    }

                    temp_second = *pText;
                }
            }

            second = temp_second;
        }
    }

    void ConvertSpecialChars(char *source)
    {
        string_view source_view(source);
        vector<wchar_t> wideText;

        if (all_of(source_view.begin(), source_view.end(), isascii))
        {
            return;
        }

        utf8::utf8to16(source_view.begin(), source_view.end(), back_inserter(wideText));

        for (wchar_t &cp : wideText)
        {
            switch (cp)
            {
            case 0xA3:
            case 0xA4:
            case 0xA7:
                cp -= 0xA0;
                break;

            default:
                break;
            }
        }

        wideText.push_back(0);
        utf8::utf16to8(wideText.begin(), wideText.end(), source);
    }

    struct CReader_ReadSimpleStatement_0x161_7
    {
        void operator()(injector::reg_pack *regs) const
        {
            CToken *pSrcToken = (CToken *)(regs->esi);
            CToken *pDstToken = (CToken *)(regs->edi);

            ConvertSpecialChars(pSrcToken->_szVal);

            *pDstToken = *pSrcToken;
        }
    };
    
    struct CSdlEvents_HandlePdxEvents_0x2DE
    {
        void operator()(injector::reg_pack *regs) const
        {
            CInputEvent temp;
            char *pText = (char *)(regs->ebp - 0x48);

            string_view text_view(pText);

            for (char c : text_view)
            {
                temp.Init(c);

                injector::thiscall<void(uint32_t, const CInputEvent *)>::vtbl<3>(regs->ebx, &temp);
            }

            memset(pText, 0, 32);

            regs->ecx = 0;
            regs->eax = 0;
        }
    };

    void InitAndPatch()
    {
        //yml转码函数
        injector::MakeJMP(g_pattern.find_pattern("81 EC B0 00 00 00 53 56 8B F1 8B DA").get(0).integer(-0x18), ConvertUTF8ToLatin1);

        //转换txt文本中的A3A4A7
        g_pattern.find_pattern("C3 8B 73 1C 8D BB 30 04 00 00 83 C6 0C B9 82 00 00 00 F3 A5"); //mov ecx, 0x82; rep movsd
        injector::MakeInline<CReader_ReadSimpleStatement_0x161_7>(g_pattern.get(0).integer(13), g_pattern.get(0).integer(20));

        //从输入法接受整个字符串
        g_pattern.find_pattern("8B 4D B8 32 C0"); //mov ecx, [ebp - 0x48]; xor al, al
        injector::MakeInline<CSdlEvents_HandlePdxEvents_0x2DE>(g_pattern.get(0).integer());

        //WriteVariable 0xA7
        injector::WriteMemory<uint8_t>(g_pattern.find_pattern("C6 06 A7").get(0).integer(2), 7, true);
        injector::WriteMemory<uint8_t>(g_pattern.find_pattern("66 C7 06 A7 21").get(0).integer(3), 7, true);

    }
}
