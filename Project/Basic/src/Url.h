#pragma once
#include <string>
#include "Basic.h"

NAMESPACE_BASIC_BEGIN
class Url
{
    static int htoi(char *s)
    {
        int value;
        int c;

        c = ((unsigned char *)s)[0];
        if (isupper(c))
            c = tolower(c);
        value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

        c = ((unsigned char *)s)[1];
        if (isupper(c))
            c = tolower(c);
        value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

        return (value);
    }

public:
    static std::string Decode(const std::string &str_source)
    {
        char const *in_str = str_source.c_str();
        int in_str_len = strlen(in_str);
        std::string out_str;
        char *str;

        str = strdup(in_str);

        char *dest = str;
        char *data = str;

        while (in_str_len--)
        {
            if (*data == '+')
                *dest = ' ';
            else if (*data == '%' && in_str_len >= 2
                && isxdigit((int) *(data + 1))
                && isxdigit((int) *(data + 2)))
            {
                *dest = (char)htoi(data + 1);
                data += 2;
                in_str_len -= 2;
            }
            else
                *dest = *data;
            ++data;
            ++dest;
        }
        *dest = '\0';
        out_str = str;
        free(str);
        return out_str;
    }

    static std::string Encode(const std::string &str_source)
    {
        char const *in_str = str_source.c_str();
        int in_str_len = strlen(in_str);
        std::string out_str;
#if __cplusplus > 199711L
        unsigned char c;
#else
        register unsigned char c;
#endif
        unsigned char *to, *start;
        unsigned char const *from, *end;
        unsigned char hexchars[] = "0123456789ABCDEF";

        from = (unsigned char *)in_str;
        end = (unsigned char *)in_str + in_str_len;
        start = to = (unsigned char *)malloc(3 * in_str_len + 1);

        while (from < end)
        {
            c = *from++;
            if (c == ' ')
                *to++ = '+';
            else if ((c < '0' && c != '-' && c != '.')
                || (c < 'A' && c > '9')
                || (c > 'Z' && c < 'a' && c != '_')
                || (c > 'z'))
            {
                to[0] = '%';
                to[1] = hexchars[c >> 4]; // equal hexchars[c / 16];
                to[2] = hexchars[c & 15]; // equal hexchars[c % 16];
                to += 3;
            }
            else
                *to++ = c;
        }
        *to = 0;
        out_str = (char *)start;
        free(start);
        return out_str;
    }

};
NAMESPACE_BASIC_END
