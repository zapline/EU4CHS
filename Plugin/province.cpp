﻿#include "province.h"
#include "stdinc.h"
#include "bitmapfont.h"
#include "functions.h"
#include "std_string.h"
#include "context.h"

//CGenerateNamesWork_AddNameArea
//InsertSpace
//51 8D 45 F0 50 8D 8D 68 FF FF FF E8 ? ? ? ? 8B 45 AC
//16
//0xD08289
struct CGenerateNamesWork_AddNameArea_InsertSpace
{
    void operator()(injector::reg_pack &regs) const
    {
        const CString *pOriginal = (const CString *)(regs.ebp.i - 0xD0);
        CString *pResult = (CString *)(regs.ebp.i - 0xA4);

        std::vector<uint32_t> wstr;
        std::string bstr;
        
        std::string_view source{ pOriginal->c_str() };
        eu4utf8::utf8to32(source.begin(), source.end(), std::back_inserter(wstr));

        for (uint32_t code : wstr)
        {
            eu4utf8::append(code, std::back_inserter(bstr));
            bstr.push_back(' ');
        }

        bstr.pop_back();

        pResult->assign(bstr.c_str());
    }
};

//CGenerateNamesWork_AddNameArea
//CountDrawables
//0F B6 04 08 8B 04 82
//7
//0xD086A0
struct CGenerateNamesWork_AddNameArea_CountDrawables
{
    void operator()(injector::reg_pack &regs) const
    {
        const CString *pString = (const CString *)(regs.ebp.i - 0xD0);
        CBitmapCharacterSet *pSet = regs.edx;

        Functions::GetTwoUnicode(
            pString->c_str(),
            regs.ecx.i,
            pString->length(),
            g_context.unicode,
            g_context.unicodeLength,
            g_context.nextUnicode,
            false);

        regs.eax.p = pSet->GetCharacterValue(g_context.unicode);
        regs.ecx.i += (g_context.unicodeLength - 1);
    }
};

//CurveText
//CalcExtraWidth
//8B 79 10 83 FF 02
//8
//0xD065A0
struct CurveText_CalcExtraWidth
{
    void operator()(injector::reg_pack &regs) const
    {
        const CString *pString = regs.ecx;
        std::string_view view(pString->c_str());

        regs.edi.i = utf8::distance(view.begin(), view.end());

        regs.ef.zero_flag = (regs.edi.i == 2);
    }
};


//CurveText
//GetCharInfo
//8A 04 01 F3 0F 10 82 28 04 00 00
//3 + 6 +3
//0xD065C9
struct CurveText_GetCharInfo
{
    void operator()(injector::reg_pack &regs) const
    {
        CString *pString = *(CString **)(regs.ebp.i - 0xDC);
        CBitmapCharacterSet *pSet = regs.edx;

        Functions::GetTwoUnicode(
            regs.ecx,
            regs.eax.i,
            pString->length(),
            g_context.unicode,
            g_context.unicodeLength,
            g_context.nextUnicode,
            false);

        regs.eax.i += (g_context.unicodeLength - 1);
        *(uint32_t *)(regs.ebp.i - 0x20) = regs.eax.i;

        regs.eax.p = pSet->GetCharacterValue(g_context.unicode);
    }
};

//CurveText
//CheckDots
//3C 27 74 0D 3C 2E
//6
//0xD06A4D
struct CurveText_CheckDots
{
    void operator()(injector::reg_pack &regs) const
    {
        regs.ef.zero_flag = (g_context.unicode == 0x27 || g_context.unicode == 0x2E);
    }
};

//CCountryNameCollection_AddNudgedNames
//CountDrawables
//0F B6 04 08 8B 84 86 B4 00 00 00
//11
//0xD08BFF
struct CCountryNameCollection_AddNudgedNames_CountDrawables
{
    void operator()(injector::reg_pack &regs) const
    {
        CBitmapFont *pFont = regs.esi;
        const CString *pString = regs.edi;

        Functions::GetTwoUnicode(
            regs.eax,
            regs.ecx.i,
            pString->length(),
            g_context.unicode,
            g_context.unicodeLength,
            g_context.nextUnicode,
            false);

        regs.eax.p = pFont->GetCharacterValue(g_context.unicode);
        regs.ecx.i += (g_context.unicodeLength - 1);
    }
};

//CCountryNameCollection_AddNudgedNames
//USAGE
//PATTERN
//NOPLENGTH
//ADDRESS
struct CCountryNameCollection_AddNudgedNames_USAGE
{
    void operator()(injector::reg_pack &regs) const
    {

    }
};

namespace Province
{
    void InitAndPatch()
    {
        g_pattern.find_pattern("51 8D 45 F0 50 8D 8D 68 FF FF FF E8 ? ? ? ? 8B 45 AC");
        if (g_pattern.has_size(1))
            injector::MakeInline<CGenerateNamesWork_AddNameArea_InsertSpace>(g_pattern.get_first().pointer(), g_pattern.get_first().pointer(16));

        g_pattern.find_pattern("0F B6 04 08 8B 04 82");
        if (g_pattern.has_size(1))
            injector::MakeInline<CGenerateNamesWork_AddNameArea_CountDrawables>(g_pattern.get_first().pointer(), g_pattern.get_first().pointer(7));

        //CurveText
        g_pattern.find_pattern("8B 79 10 83 FF 02");
        if (g_pattern.has_size(1))
            injector::MakeInline<CurveText_CalcExtraWidth>(g_pattern.get_first().pointer(), g_pattern.get_first().pointer(8));

        g_pattern.find_pattern("8A 04 01 F3 0F 10 82 28 04 00 00");
        if (g_pattern.has_size(1))
        {
            injector::MakeNOP(g_pattern.get_first().pointer(), 3);
            injector::MakeNOP(g_pattern.get_first().pointer(11), 6);
            injector::MakeNOP(g_pattern.get_first().pointer(22), 3);
            injector::MakeInline<CurveText_GetCharInfo>(g_pattern.get_first().pointer(11));
        }

        g_pattern.find_pattern("3C 27 74 0D 3C 2E");
        if (g_pattern.has_size(1))
            injector::MakeInline<CurveText_CheckDots>(g_pattern.get_first().pointer(), g_pattern.get_first().pointer(6));

        //CCountryNameCollection::AddNudgedNames
        g_pattern.find_pattern("0F B6 04 08 8B 84 86 B4 00 00 00");
        if (g_pattern.has_size(1))
            injector::MakeInline<CCountryNameCollection_AddNudgedNames_CountDrawables>(g_pattern.get_first().pointer(), g_pattern.get_first().pointer(11));
    }
}
