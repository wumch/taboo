
#pragma once

#include "predef.hpp"
#include <cstring>
extern "C" {
#   include <openssl/md5.h>
}

namespace taboo
{

class StrRef
{
private:
    const char* _data;
    std::size_t len;

public:
    StrRef():
        _data(NULL), len(0)
    {}

    explicit StrRef(const char* __data, std::size_t _len):
        _data(__data), len(_len)
    {}

    const char* data() const
    {
        return _data;
    }

    std::size_t size() const
    {
        return len;
    }

    std::size_t length() const
    {
        return size();
    }
};

inline bool operator<(const StrRef& lhs, const StrRef& rhs)
{
    int nres = std::strncmp(lhs.data(), rhs.data(), std::min(lhs.length(), rhs.length()));
    return (nres == 0) ? lhs.length() < rhs.length() : (nres == -1);
}

inline bool operator==(const std::string& str, const StrRef& ref)
{
    return str.length() == ref.length() && !std::strncmp(str.data(), ref.data(), ref.length());
}

inline bool operator==(const StrRef& ref, const std::string& str)
{
    return str == ref;
}

namespace
{

class Assister
{
public:
    static char hex(uint8_t chr) CS_ATTR_CONST
    {
        return chr + ((chr < 10) ? '0' : ('a' - 10));
    }

    static std::string hex(const uint8_t* data, std::size_t length)
    {
        std::string res;
        res.resize(length << 1);
        hex(data, length, res.begin(), res.end());
        return res;
    }

    template<typename It>
    static bool hex(const uint8_t* data, std::size_t length, It begin, const It end)
    {
        if (CS_UNLIKELY(end - begin < static_cast<int64_t>(length << 1))) {
            return false;
        }
        return hex(data, length, begin);
    }

    // NOTE: 外部保证 begin 足够长，否则溢出。
    template<typename It>
    static bool hex(const uint8_t* data, std::size_t length, It begin)
    {
        for (const uint8_t* dataEnd = data + length; data != dataEnd; ++data)
        {
            *begin++ = hex(*data >> 4);
            *begin++ = hex(*data & 0x0f);
        }
        return true;
    }

    static char hexUpper(uint8_t chr) CS_ATTR_CONST
    {
        return chr + ((chr < 10) ? '0' : ('A' - 10));
    }
};

}

class MD5Stream
{
    friend MD5Stream& operator>>(MD5Stream&, char*);
    friend MD5Stream& operator>>(MD5Stream&, std::string&);
private:
    MD5_CTX ctx;
    bool finished, ok;

public:
    enum { RESULT_SIZE = MD5_DIGEST_LENGTH << 1 };

    MD5Stream():
        finished(false), ok(true)
    {
        MD5_Init(&ctx);
    }

    const std::string hex()
    {
        if (finished) {
            return std::string();
        }
        uint8_t binary[MD5_DIGEST_LENGTH];
        MD5_Final(binary, &ctx);
        finished = true;
        return Assister::hex(binary, sizeof(binary));
    }

    template<typename It>
    const bool hex(It begin, const It end)
    {
        if (finished) {
            return false;
        }
        uint8_t binary[MD5_DIGEST_LENGTH];
        MD5_Final(binary, &ctx);
        finished = true;
        return Assister::hex(binary, sizeof(binary), begin, end);
    }

    // NOTE: 外部保证 begin 足够长，否则溢出。
    template<typename It>
    const bool hex(It begin)
    {
        if (finished) {
            return false;
        }
        uint8_t binary[MD5_DIGEST_LENGTH];
        MD5_Final(binary, &ctx);
        finished = true;
        return Assister::hex(binary, sizeof(binary), begin);
    }

    MD5Stream& append(const char* data, std::size_t len)
    {
        if (finished) { ok = false; }
        if (len) {
            MD5_Update(&ctx, data, len);
        }
        return *this;
    }

    bool good() const
    {
        return ok;
    }

    ~MD5Stream()
    {
        if (!finished)
        {
            uint8_t binary[MD5_DIGEST_LENGTH];
            MD5_Final(binary, &ctx);
        }
    }
};

inline MD5Stream& operator<<(MD5Stream& stream, const std::string& data)
{
    stream.append(data.data(), data.size());
    return stream;
}

inline MD5Stream& operator<<(MD5Stream& stream, const char* data)
{
    stream.append(data, std::strlen(data));
    return stream;
}

inline MD5Stream& operator<<(MD5Stream& stream, const StrRef& str)
{
    stream.append(str.data(), str.length());
    return stream;
}

inline MD5Stream& operator<<(MD5Stream& stream, const char data)
{
    stream.append(&data, sizeof(char));
    return stream;
}

inline MD5Stream& operator>>(MD5Stream& stream, std::string& str)
{
    if (str.empty())
    {
        str.resize(MD5Stream::RESULT_SIZE);
    }
    stream.ok = stream.hex(str.begin(), str.end());
    return stream;
}

// NOTE: 外部保证 begin 足够长，否则溢出。
inline MD5Stream& operator>>(MD5Stream& stream, char* str)
{
    stream.ok = stream.hex(str);
    return stream;
}

}
