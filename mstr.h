#pragma once

#include <string>

template <typename T_CHAR>
inline void mstr_trim(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t i = str.find_first_not_of(spaces);
    size_t j = str.find_last_not_of(spaces);
    if ((i == string_type::npos) || (j == string_type::npos))
        str.clear();
    else
        str = str.substr(i, j - i + 1);
}

template <typename T_CHAR>
inline void mstr_trim_left(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t i = str.find_first_not_of(spaces);
    if (i == string_type::npos)
        str.clear();
    else
        str = str.substr(i);
}

template <typename T_CHAR>
inline void mstr_trim_right(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t j = str.find_last_not_of(spaces);
    if (j == string_type::npos)
        str.clear();
    else
        str = str.substr(0, j + 1);
}

template <typename T_STR>
inline bool
mstr_replace_all(T_STR& str, const T_STR& from, const T_STR& to)
{
    bool found = false;
    size_t i = 0;
    for (;;)
    {
        i = str.find(from, i);
        if (i == T_STR::npos)
            break;
        found = true;
        str.replace(i, from.size(), to);
        i += to.size();
    }
    return found;
}
template <typename T_STR>
inline bool
mstr_replace_all(T_STR& str,
                 const typename T_STR::value_type *from,
                 const typename T_STR::value_type *to)
{
    return mstr_replace_all(str, T_STR(from), T_STR(to));
}
template <typename T_STR>
inline bool
mstr_replace_all(T_STR& str, typename T_STR::value_type from, typename T_STR::value_type to)
{
    bool found = false;
    for (auto& ch : str)
    {
        if (ch == from)
        {
            ch = to;
            found = true;
        }
    }
    return found;
}

template <typename T_STR_CONTAINER>
inline void
mstr_split(T_STR_CONTAINER& container,
           const typename T_STR_CONTAINER::value_type& str,
           const typename T_STR_CONTAINER::value_type& chars)
{
    container.clear();
    size_t i = 0, k = str.find_first_of(chars);
    while (k != T_STR_CONTAINER::value_type::npos)
    {
        container.push_back(str.substr(i, k - i));
        i = k + 1;
        k = str.find_first_of(chars, i);
    }
    container.push_back(str.substr(i));
}

template <typename T_STR_CONTAINER>
inline typename T_STR_CONTAINER::value_type
mstr_join(const T_STR_CONTAINER& container,
          const typename T_STR_CONTAINER::value_type& sep)
{
    typename T_STR_CONTAINER::value_type result;
    typename T_STR_CONTAINER::const_iterator it, end;
    it = container.begin();
    end = container.end();
    if (it != end)
    {
        result = *it;
        for (++it; it != end; ++it)
        {
            result += sep;
            result += *it;
        }
    }
    return result;
}
