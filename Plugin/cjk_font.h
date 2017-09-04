﻿#pragma once
#include "stdinc.h"
#include "eu4.h"

class CJKFont
{
public:
    struct CharacterValues
    {
        EU4CharacterValues EU4Values;
        std::uint16_t PageIndex;
    };

    static const std::uint32_t invalid_replacement = L'？';

    CJKFont(const std::experimental::filesystem::path &fntname);

    void InitWithFile(const std::experimental::filesystem::path &fntname);

    void LoadTexturesDX9();
    void UnloadTexturesDX9();

    const CharacterValues *GetValue(std::uint32_t unicode);
    std::int16_t GetKerning(uint32_t first, uint32_t second) const;
    TextureGFX *GetTexture(std::uint32_t unicode);

private:
    bool _initialized;
    std::experimental::filesystem::path _workingdir;
    std::uint16_t _scaleW;
    std::uint16_t _scaleH;
    std::uint16_t _pages;
    std::unordered_map<std::uint32_t, CharacterValues> _values;
    std::unordered_map<std::uint64_t, std::int16_t> _kernings;
    std::vector<TextureGFX> _textures;
    std::vector<std::string> _texturenames;

    void ReadInfoBlock(FILE *file);
    void ReadCommonBlock(FILE *file);
    void ReadPagesBlock(FILE *file);
    void ReadCharsBlock(FILE *file);
    void ReadKerningsBlock(FILE *file);
    void SetKernings();
};
