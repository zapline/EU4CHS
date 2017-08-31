﻿#include "byte_pattern.h"
#include <stdexcept>

extern const HMODULE pattern_default_module = GetModuleHandleA(NULL);
byte_pattern g_pattern;

const memory_pointer &byte_pattern::get(std::size_t index) const
{
    if (index >= this->_results.size())
    {
        std::stringstream sstr;

        sstr << "Processing pattern: " << this->_literal << "\nTrying to access index " << index << " but only " << this->_results.size() << " results.\nGame will crash.";

        MessageBoxA(NULL, sstr.str().c_str(), "byte_pattern: too few results.", MB_OK);

        throw std::out_of_range{ "Pattern: results accessing out of range." };
    }

    return this->_results[index];
}

byte_pattern::byte_pattern()
{
    set_module();
}

byte_pattern &byte_pattern::set_pattern(const char *pattern_literal)
{
    this->_results.clear();
    this->_processed = false;
    this->transform_pattern(pattern_literal);

    return *this;
}

byte_pattern &byte_pattern::set_pattern(const void *data, std::size_t size)
{
    this->_pattern.assign(reinterpret_cast<const std::uint8_t *>(data), reinterpret_cast<const std::uint8_t *>(data) + size);
    this->bm_preprocess();

    return *this;
}

byte_pattern &byte_pattern::set_module(memory_pointer module)
{
    this->get_module_range(module);

    return *this;
}

byte_pattern &byte_pattern::set_range(memory_pointer beg, memory_pointer end)
{
    this->_range = { beg.address(), end.address() };

    return *this;
}

void byte_pattern::do_search()
{
    if (!this->_processed)
    {
        this->force_search();
    }
}

byte_pattern &byte_pattern::force_search()
{
    this->_results.clear();

    this->_processed = true;

    if (!this->_pattern.empty())
    {
        this->bm_search();
    }

    return *this;
}

byte_pattern & byte_pattern::find_pattern(const char * pattern_literal)
{
    this->set_pattern(pattern_literal).force_search();

    return *this;
}

void byte_pattern::transform_pattern(const char *pattern_literal)
{
    auto tol = [](char ch) -> uint8_t
    {
        if (ch >= 'A' && ch <= 'F') return uint8_t(ch - 'A' + 10);
        if (ch >= 'a' && ch <= 'f') return uint8_t(ch - 'a' + 10);
        return uint8_t(ch - '0');
    };

    auto is_digit = [](char ch) -> bool
    {
        return (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f') || (ch >= '0' && ch <= '9');
    };

    std::array<char, 2> temp_string{ 0, 0 };

    if (pattern_literal == nullptr)
    {
        return;
    }

    this->_literal = pattern_literal;

    this->_pattern.clear();
    this->_mask.clear();

    const char *patit = pattern_literal;
    const char *patend = (pattern_literal + std::strlen(pattern_literal) + 1);

    while (patit != patend)
    {
        char ch = *patit;

        if (ch == ' ' || ch == 0)
        {
            if (!temp_string[0] && !temp_string[1])
            {
                continue;
            }
            else if (temp_string[0] == '?' && (temp_string[1] == '?' || temp_string[1] == 0))
            {
                this->_pattern.emplace_back(0);
                this->_mask.emplace_back(0x00u);
            }
            else if (temp_string[0] == '?' && is_digit(temp_string[1]))
            {
                this->_pattern.emplace_back(tol(temp_string[1]));
                this->_mask.emplace_back(0x0Fu);
            }
            else if (temp_string[1] == '?' && is_digit(temp_string[0]))
            {
                this->_pattern.emplace_back(tol(temp_string[0]) << 4);
                this->_mask.emplace_back(0xF0u);
            }
            else if (is_digit(temp_string[0]) && is_digit(temp_string[1]))
            {
                this->_pattern.emplace_back((tol(temp_string[0]) << 4) | tol(temp_string[1]));
                this->_mask.emplace_back(0xFFu);
            }
            else
            {
                this->_pattern.clear();
                this->_mask.clear();
                return;
            }

            temp_string.fill(0);
        }
        else
        {
            if (temp_string[0] == 0)
            {
                temp_string[0] = ch;
            }
            else if (temp_string[1] == 0)
            {
                temp_string[1] = ch;
            }
            else
            {
                this->_pattern.clear();
                this->_mask.clear();
                return;
            }
        }

        ++patit;
    }

    this->bm_preprocess();
}

void byte_pattern::get_module_range(memory_pointer module)
{
    static auto getSection = [](const PIMAGE_NT_HEADERS nt_headers, unsigned section) -> PIMAGE_SECTION_HEADER
    {
        return reinterpret_cast<PIMAGE_SECTION_HEADER>(
            (UCHAR*)nt_headers->OptionalHeader.DataDirectory +
            nt_headers->OptionalHeader.NumberOfRvaAndSizes * sizeof(IMAGE_DATA_DIRECTORY) +
            section * sizeof(IMAGE_SECTION_HEADER));
    };

    this->_range.first = module.address();

    PIMAGE_DOS_HEADER dosHeader = module.pointer<IMAGE_DOS_HEADER>();
    PIMAGE_NT_HEADERS ntHeader = module.pointer<IMAGE_NT_HEADERS>(dosHeader->e_lfanew);

    for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
    {
        auto sec = getSection(ntHeader, i);
        auto secSize = sec->SizeOfRawData != 0 ? sec->SizeOfRawData : sec->Misc.VirtualSize;
        if (sec->Characteristics & IMAGE_SCN_MEM_EXECUTE)
            this->_range.second = this->_range.first + sec->VirtualAddress + secSize;

        if ((i == ntHeader->FileHeader.NumberOfSections - 1) && this->_range.second == 0)
            this->_range.second = this->_range.first + sec->PointerToRawData + secSize;
    }
}

void byte_pattern::clear()
{
    _range = { 0,0 };
    this->_pattern.clear();
    this->_mask.clear();
    this->_bmgs.clear();
    this->_results.clear();
    this->_processed = false;
}

std::size_t byte_pattern::size() const
{
    return this->_results.size();
}

bool byte_pattern::has_size(std::size_t expected) const
{
    return (this->_results.size() == expected);
}

bool byte_pattern::empty() const
{
    return this->_results.empty();
}

void byte_pattern::bm_preprocess()
{
    std::ptrdiff_t index;

    const std::uint8_t *pbytes = this->_pattern.data();
    const std::uint8_t *pmask = this->_mask.data();
    std::size_t pattern_len = this->_pattern.size();

    for (std::uint32_t bc = 0; bc < 256; ++bc)
    {
        for (index = pattern_len - 1; index >= 0; --index)
        {
            if ((pbytes[index] & pmask[index]) == (bc & pmask[index]))
            {
                break;
            }
        }

        this->_bmbc[bc] = index;
    }

    this->_bmgs.resize(pattern_len, 1);
}

void byte_pattern::bm_search()
{
    const std::uint8_t *pbytes = this->_pattern.data();
    const std::uint8_t *pmask = this->_mask.data();
    const std::ptrdiff_t *pgoodsuffix = this->_bmgs.data();
    std::size_t pattern_len = this->_pattern.size();

    std::uint8_t *range_begin = reinterpret_cast<std::uint8_t *>(this->_range.first);
    std::uint8_t *range_end = reinterpret_cast<std::uint8_t *>(this->_range.second - pattern_len);

    std::ptrdiff_t index;

    __try
    {
        while (range_begin <= range_end)
        {
            for (index = pattern_len - 1; index >= 0; --index)
            {
                if ((pbytes[index] & pmask[index]) != (range_begin[index] & pmask[index]))
                {
                    break;
                }
            }

            if (index == -1)
            {
                this->_results.emplace_back(range_begin);
                range_begin += pattern_len;
            }
            else
            {
                range_begin += max(index - this->_bmbc[range_begin[index]], pgoodsuffix[index]);
            }
        }
    }
    __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {

    }
}