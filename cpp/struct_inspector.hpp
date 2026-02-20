#ifndef CONST_STRUCT_INSPECTOR_HPP
#define CONST_STRUCT_INSPECTOR_HPP

#include <string>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <sstream>
#include <cstdint>
#include <utility>

#include "inspector_dsl.hpp"

class ConstStructInspector
{
public:
    ConstStructInspector() = default;

    std::vector<std::string> names() const;

    bool contains(const std::string& name) const;

    const std::type_index type_id(const std::string& name) const;

    std::string str(const std::string& name, const char* fmt = nullptr) const;

    std::string type(const std::string& name) const;

    template <class T, class... Indices>
    void add(const char* expr, const T& ref, Indices&&... indices)
    {
        const std::string key = build_key(expr, std::forward<Indices>(indices)...);
        add_impl(key, const_cast<T*>(&ref), std::type_index(typeid(T)));
    }

    template <typename T>
    const T* get(const std::string& name) const
    {
        auto it = slots_.find(name);
        if (it == slots_.end())
            return nullptr;
        const auto& s = it->second;
        if (s.ti != std::type_index(typeid(T)))
            return nullptr;
        return static_cast<const T*>(s.ptr);
    }

    template <typename U>
    void register_leaf(const std::string& key, const U* p)
    {
        add_impl(key, const_cast<U*>(p), std::type_index(typeid(U)));
    }

    struct RegisterLeafVisitor 
    {
        ConstStructInspector* self;
        template <typename U>
        void operator()(const std::string& key, const U* p) const 
        {
            self->register_leaf(key, p);
        }
    };

    template <typename T>
    void bind_struct(const T& root, const std::string& prefix = std::string())
    {
        RegisterLeafVisitor visitor{ this };
        inspect_recursive(root, prefix, visitor);
    }

private:
    struct Slot
    {
        void* ptr = nullptr;
        std::type_index ti = std::type_index(typeid(void));
    };

    std::unordered_map<std::string, Slot> slots_;
    std::unordered_map<std::type_index, std::vector<std::string>> type_index_;

    void add_impl(const std::string& key, void* ptr, std::type_index ti);

    static std::string build_key(const char* expr_literal); 

    template <class... Indices>
    static std::string build_key(const char* expr_literal, Indices&&... indices)
    {
        auto args = pack_indices(std::forward<Indices>(indices)...);
        return parse_key_expr(expr_literal, args);
    }

    static std::string parse_key_expr(const char* expr_literal, const std::vector<std::string>& args);
    static std::string quote_and_escape(const std::string& s); 
    static std::string to_index_string(const std::string& s);  
    static std::string to_index_string(const char* s);         
    static std::string to_index_string(char* s);               

    template <class U>
    static std::string to_index_string(const U& v)
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }

    template <class... Indices>
    static std::vector<std::string> pack_indices(Indices&&... indices)
    {
        std::vector<std::string> out;
        out.reserve(sizeof...(Indices));
        int dummy[] = { 0, (out.push_back(to_index_string(std::forward<Indices>(indices))), 0)... };
        (void)dummy;
        return out;
    }

    static std::string snprintf_one(const char* fmt, long long v);            
    static std::string snprintf_one(const char* fmt, unsigned long long v);   
    static std::string snprintf_one(const char* fmt, double v);               
    static std::string snprintf_one(const char* fmt, const char* s);          

    static std::string format_value_signed(long long v, const char* fmt);             
    static std::string format_value_unsigned(unsigned long long v, const char* fmt);  
    static std::string format_value_float(double v, const char* fmt);                 
    static std::string format_value_string(const std::string& s, const char* fmt);    
    static std::string format_value_cstr(const char* s, const char* fmt);             
    static std::string format_value_bool(bool v, const char* fmt);                    
    static std::string format_value_stream(const std::string&, const std::type_index&, const void*, const char* fmt); 
};

#endif