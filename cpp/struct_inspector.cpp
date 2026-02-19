#include <string>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <sstream>
#include <cstdint>
#include <utility>
#include <cstdio>
#include <limits>
#include <sstream>

#include "struct_inspector.hpp"

std::vector<std::string> ConstStructInspector::names() const
{
    std::vector<std::string> v;
    v.reserve(slots_.size());
    for (const auto& kv : slots_)
        v.push_back(kv.first);
    return v;
}

bool ConstStructInspector::contains(const std::string& name) const
{
    return slots_.find(name) != slots_.end();
}

const std::type_index ConstStructInspector::type_id(const std::string& name) const
{
    auto it = slots_.find(name);
    if (it == slots_.end())
        return std::type_index(typeid(void)); // 見つからない → void
    return it->second.ti;                      // 見つかった → 実タイプ
}

std::string ConstStructInspector::str(const std::string& name, const char* fmt) const
{
    auto it = slots_.find(name);
    if (it == slots_.end())
        return std::string();

    const Slot& s = it->second;
    const std::type_index& ti = s.ti;
    const void* p = s.ptr;

    // 整数
    if (ti == typeid(bool))              return format_value_bool(*static_cast<const bool*>(p), fmt);

    if (ti == typeid(char))              return format_value_signed((long long)*static_cast<const char*>(p), fmt);
    if (ti == typeid(signed char))       return format_value_signed((long long)*static_cast<const signed char*>(p), fmt);
    if (ti == typeid(short))             return format_value_signed((long long)*static_cast<const short*>(p), fmt);
    if (ti == typeid(int))               return format_value_signed((long long)*static_cast<const int*>(p), fmt);
    if (ti == typeid(long))              return format_value_signed((long long)*static_cast<const long*>(p), fmt);
    if (ti == typeid(long long))         return format_value_signed(*static_cast<const long long*>(p), fmt);

    if (ti == typeid(unsigned char))     return format_value_unsigned((unsigned long long)*static_cast<const unsigned char*>(p), fmt);
    if (ti == typeid(uint16_t))          return format_value_unsigned((unsigned long long)*static_cast<const uint16_t*>(p), fmt);
    if (ti == typeid(unsigned short))    return format_value_unsigned((unsigned long long)*static_cast<const unsigned short*>(p), fmt);
    if (ti == typeid(unsigned int))      return format_value_unsigned((unsigned long long)*static_cast<const unsigned int*>(p), fmt);
    if (ti == typeid(unsigned long))     return format_value_unsigned((unsigned long long)*static_cast<const unsigned long*>(p), fmt);
    if (ti == typeid(unsigned long long))return format_value_unsigned(*static_cast<const unsigned long long*>(p), fmt);

    // 浮動小数
    if (ti == typeid(float))             return format_value_float((double)*static_cast<const float*>(p), fmt);
    if (ti == typeid(double))            return format_value_float(*static_cast<const double*>(p), fmt);
    if (ti == typeid(long double))       return format_value_float((double)*static_cast<const long double*>(p), fmt);

    // 文字列
    if (ti == typeid(std::string))       return format_value_string(*static_cast<const std::string*>(p), fmt);
    if (ti == typeid(const char*))       return format_value_cstr(*static_cast<const char* const*>(p), fmt);
    if (ti == typeid(char*))             return format_value_cstr(*static_cast<char* const*>(p), fmt);

    // その他
    return format_value_stream(name, ti, p, fmt);
}

std::string ConstStructInspector::type(const std::string& name) const
{
    auto it = slots_.find(name);
    if (it == slots_.end()) return std::string();

    const std::type_index& ti = it->second.ti;

    // ブール
    if (ti == typeid(bool))              return "bool";

    // 符号付き整数
    if (ti == typeid(int8_t))            return "i8";
    if (ti == typeid(char))              return "i8";
    if (ti == typeid(signed char))       return "i8";
    if (ti == typeid(int16_t))           return "i16";
    if (ti == typeid(short))             return "i16";
    if (ti == typeid(int32_t))           return "i32";
    if (ti == typeid(int))               return "i32";
    if (ti == typeid(long))              return sizeof(long) == 8 ? "i64" : "i32";
    if (ti == typeid(int64_t))           return "i64";
    if (ti == typeid(long long))         return "i64";

    // 符号なし整数
    if (ti == typeid(uint8_t))           return "u8";
    if (ti == typeid(unsigned char))     return "u8";
    if (ti == typeid(uint16_t))          return "u16";
    if (ti == typeid(unsigned short))    return "u16";
    if (ti == typeid(unsigned int))      return "u32";
    if (ti == typeid(uint32_t))          return "u32";
    if (ti == typeid(unsigned long))     return sizeof(unsigned long) == 8 ? "u64" : "u32";
    if (ti == typeid(uint64_t))          return "u64";
    if (ti == typeid(unsigned long long))return "u64";

    // 浮動小数
    if (ti == typeid(float))             return "f32";
    if (ti == typeid(double))            return "f64";
    if (ti == typeid(long double))       return "f80"; // 必要に応じて調整

    // 文字列
    if (ti == typeid(std::string))       return "string";
    if (ti == typeid(const char*))       return "cstr";
    if (ti == typeid(char*))             return "cstr";

    return std::string();
}

void ConstStructInspector::add_impl(const std::string& key, void* ptr, std::type_index ti)
{
    Slot slot;
    slot.ptr = ptr;
    slot.ti  = ti;
    slots_[key] = slot;
    type_index_[ti].push_back(key);
}

std::string ConstStructInspector::build_key(const char* expr_literal)
{
    return std::string(expr_literal);
}

std::string ConstStructInspector::parse_key_expr(const char* expr_literal, const std::vector<std::string>& args)
{
    std::string out;
    out.reserve(64);

    const char* p = expr_literal;
    std::size_t idx = 0;

    while (*p)
    {
        if (*p == '[')
        {
            const char* q = p + 1;
            while (*q && *q != ']') ++q;

            if (*q == ']')
            {
                out.push_back('[');

                if (idx < args.size())
                    out += args[idx++];
                else
                    out.append(p + 1, static_cast<std::size_t>(q - (p + 1)));

                out.push_back(']');
                p = q + 1;
                continue;
            }
        }
        out.push_back(*p++);
    }

    return out;
}

std::string ConstStructInspector::quote_and_escape(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (char c : s)
    {
        switch (c)
        {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out.push_back(c); break;
        }
    }
    out.push_back('"');
    return out;
}

std::string ConstStructInspector::to_index_string(const std::string& s)
{
    return quote_and_escape(s);
}

std::string ConstStructInspector::to_index_string(const char* s)
{
    return quote_and_escape(s ? std::string(s) : std::string());
}

std::string ConstStructInspector::to_index_string(char* s)
{
    return quote_and_escape(s ? std::string(s) : std::string());
}

std::string ConstStructInspector::snprintf_one(const char* fmt, long long v)
{
    std::vector<char> buf(64);
    while (true)
    {
        int n = std::snprintf(buf.data(), buf.size(), fmt, v);
        if (n < 0) return std::string();
        if ((size_t)n < buf.size()) return std::string(buf.data(), (size_t)n);
        buf.resize((size_t)n + 1);
    }
}

std::string ConstStructInspector::snprintf_one(const char* fmt, unsigned long long v)
{
    std::vector<char> buf(64);
    while (true)
    {
        int n = std::snprintf(buf.data(), buf.size(), fmt, v);
        if (n < 0) return std::string();
        if ((size_t)n < buf.size()) return std::string(buf.data(), (size_t)n);
        buf.resize((size_t)n + 1);
    }
}

std::string ConstStructInspector::snprintf_one(const char* fmt, double v)
{
    std::vector<char> buf(64);
    while (true)
    {
        int n = std::snprintf(buf.data(), buf.size(), fmt, v);
        if (n < 0) return std::string();
        if ((size_t)n < buf.size()) return std::string(buf.data(), (size_t)n);
        buf.resize((size_t)n + 1);
    }
}

std::string ConstStructInspector::snprintf_one(const char* fmt, const char* s)
{
    std::vector<char> buf(64);
    while (true)
    {
        int n = std::snprintf(buf.data(), buf.size(), fmt, s);
        if (n < 0) return std::string();
        if ((size_t)n < buf.size()) return std::string(buf.data(), (size_t)n);
        buf.resize((size_t)n + 1);
    }
}

std::string ConstStructInspector::format_value_signed(long long v, const char* fmt)
{
    if (!fmt) { std::ostringstream oss; oss << v; return oss.str(); }
    return snprintf_one(fmt, v);
}

std::string ConstStructInspector::format_value_unsigned(unsigned long long v, const char* fmt)
{
    if (!fmt) { std::ostringstream oss; oss << v; return oss.str(); }
    return snprintf_one(fmt, v);
}

std::string ConstStructInspector::format_value_float(double v, const char* fmt)
{
    if (!fmt) { std::ostringstream oss; oss << v; return oss.str(); }
    return snprintf_one(fmt, v);
}

std::string ConstStructInspector::format_value_string(const std::string& s, const char* fmt)
{
    if (!fmt) return s;
    return snprintf_one(fmt, s.c_str());
}

std::string ConstStructInspector::format_value_cstr(const char* s, const char* fmt)
{
    if (!fmt) return std::string(s ? s : "");
    return snprintf_one(fmt, s ? s : "");
}

std::string ConstStructInspector::format_value_bool(bool v, const char* fmt)
{
    (void)fmt;
    return v ? "true" : "false";
}

std::string ConstStructInspector::format_value_stream(const std::string&, const std::type_index&, const void*, const char* fmt)
{
    (void)fmt;
    std::ostringstream oss;
    oss << "<unsupported-type>";
    return oss.str();
}
