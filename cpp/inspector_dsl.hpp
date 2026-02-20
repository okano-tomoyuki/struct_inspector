#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

template <typename T>
struct InspectorTraits
{
    static const bool defined = false;
};

template <typename T, typename F>
void inspect_recursive(T &obj, const std::string &prefix, F &visitor, typename std::enable_if<!std::is_array<T>::value && !InspectorTraits<T>::defined>::type * = 0)
{
    visitor(prefix, &obj);
}

template <typename T, typename F>
void inspect_recursive(T &obj, const std::string &prefix, F &visitor, typename std::enable_if<!std::is_array<T>::value && InspectorTraits<T>::defined>::type * = 0)
{
    InspectorTraits<T>::accept(obj, prefix, visitor);
}

template <typename T, typename F>
void inspect_recursive(T &obj, const std::string &prefix, F &visitor, typename std::enable_if<std::is_array<T>::value>::type * = 0)
{
    typedef typename std::remove_extent<T>::type ElementType;
    size_t extent = sizeof(obj) / sizeof(ElementType);

    for (size_t i = 0; i < extent; ++i)
    {
        std::string indexed_path = prefix + "[" + std::to_string(i) + "]";
        inspect_recursive(obj[i], indexed_path, visitor);
    }
}

template <typename T, typename F>
void inspect_recursive(const T &obj, const std::string &prefix, F &visitor, typename std::enable_if<!std::is_array<T>::value && !InspectorTraits<T>::defined>::type * = 0)
{
    visitor(prefix, &obj);
}

template <typename T, typename F>
void inspect_recursive(const T &obj, const std::string &prefix, F &visitor, typename std::enable_if<!std::is_array<T>::value && InspectorTraits<T>::defined>::type * = 0)
{
    InspectorTraits<T>::accept(obj, prefix, visitor);
}

template <typename T, typename F>
void inspect_recursive(const T &obj, const std::string &prefix, F &visitor, typename std::enable_if<std::is_array<T>::value>::type * = 0)
{
    typedef typename std::remove_extent<T>::type ElementType;
    size_t extent = sizeof(obj) / sizeof(ElementType);
    for (size_t i = 0; i < extent; ++i)
    {
        std::string indexed_path = prefix + "[" + std::to_string(i) + "]";
        inspect_recursive(obj[i], indexed_path, visitor);
    }
}

#define REGISTER_INSPECTOR(StructName)                                                   \
    template <>                                                                          \
    struct InspectorTraits<StructName>                                                   \
    {                                                                                    \
        static const bool defined = true;                                                \
        template <typename F>                                                            \
        static void accept(const StructName &obj, const std::string &prefix, F &visitor) \
        {                                                                                \
            typedef StructName Target;

#define FIELD(Member) inspect_recursive(obj.Member, (prefix.empty() ? #Member : prefix + "." + #Member), visitor);

#define REGISTER_END() } };