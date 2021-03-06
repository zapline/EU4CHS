﻿#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include "../include/utf8cpp/utf8.h"

using namespace std;
using namespace std::experimental::filesystem::v1;

static void CollectChars(const path &filename, set<uint32_t> &collection)
{
    ifstream stream{ filename };

    if (!stream)
    {
        return;
    }

    vector<uint32_t> wide_text;

    utf8::utf8to32(istreambuf_iterator<char>(stream), istreambuf_iterator<char>{}, back_inserter(wide_text));

    for (auto wide_char : wide_text)
    {
        if (wide_char > 0x20)
        {
            collection.insert(wide_char);
        }
    }
}

static set<uint32_t> ScanFolder(const path &folder)
{
    set<uint32_t> result;

    directory_iterator dir_it{ folder };

    while (dir_it != directory_iterator{})
    {
        path filename = dir_it->path();

        if (is_regular_file(filename) && (filename.extension() == ".yml" || filename.extension() == ".txt"))
        {
            CollectChars(filename, result);
        }

        ++dir_it;
    }

    result.erase(0xFEFF);

    return result;
}

static void GenerateTable(const set<uint32_t> &collection, const path &text)
{
    ofstream ofs(text, ios::trunc);

    if (!ofs)
    {
        return;
    }

    utf8::utf32to8(collection.begin(), collection.end(), utf8::append(0xFEFF, ostreambuf_iterator<char>(ofs)));

    ofs.close();
}

//.exe 文件夹 输出文本
int main(int argc, char **argv)
{
    if (argc == 3)
    {
        set<uint32_t> collection = ScanFolder(argv[1]);
        GenerateTable(collection, argv[2]);
    }
    else
    {
        set<uint32_t> collection = ScanFolder(R"(C:\Users\Kelashi\Desktop\EU4\Translation\全部文本)");
        GenerateTable(collection, R"(C:\Users\Kelashi\Desktop\EU4\Translation\characters.txt)");

        set<uint32_t> collection1 = ScanFolder(R"(C:\Users\Kelashi\Desktop\EU4\Translation\大地图)");
        GenerateTable(collection1, R"(C:\Users\Kelashi\Desktop\EU4\Translation\province.txt)");
    }

    return 0;
}
