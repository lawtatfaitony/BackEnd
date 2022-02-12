#pragma once
#include <string>
#include <vector>



class Hanz2Piny 
{
    public:
        typedef unsigned short Unicode;
        enum Polyphone {all, name, noname};

    public:
        // unicode
        static bool IsUnicode(const Unicode unicode);
        static std::vector<std::string> GetPinyByUnicode(const Unicode hanzi_unicode, const bool with_tone = false);
        // utf8
        static bool IsUtf8(const std::string& s);
        static std::string GetPinyByUtf8(const std::string& s, const bool with_tone = false);
        static std::vector<std::pair<bool, std::vector<std::string>>> GetPinyListByUtf8(const std::string& s,
            const bool with_tone = true,
            const bool replace_unknown = false,
            const std::string& replace_unknown_with = "");

        static bool IsUtf8File (const std::string& file_path);
        static bool IsStartWiithbom (const std::string& s);

    private:
        static const Unicode begin_hanzi_unicode_, end_hanzi_unicode_;
        static const char* pinyin_list_with_tone_[];
};