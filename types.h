// types.h --- VeySick basic types

#pragma once

#define _CRT_SECURE_NO_WARNINGS // Visual C++におけるfopenなどの警告を無視する
#define _USE_MATH_DEFINES // Visual C++でも円周率定数 M_PI を使いたい

#include <vector>           // For std::vector
#include <map>              // For std::map
#include <unordered_map>    // For std::unordered_map
#include <string>           // For std::string and std::wstring
#include <memory>           // For std::shared_ptr and std::make_shared
#include <cstring>          // For std::strlen std::memcpy etc.
#include <cassert>          // For assert macro
#include <cstdint>          // For uint8_t, uint16_t etc.
#include <cmath>            // For Math
#include <deque>            // For std::deque
#include <set>              // For std::set
#include <limits>           // For std::numeric_limits

//////////////////////////////////////////////////////////////////////////////
// VeySicK 基本型

// 基本型の定義
typedef   std::int8_t VskChar;
typedef  std::uint8_t VskByte;
typedef  std::int16_t VskShort;
typedef std::uint16_t VskWord;
typedef  std::int32_t VskLong;
typedef std::uint32_t VskDword, VskDWord;
typedef  std::int64_t VskLongLong;
typedef std::uint64_t VskDwordLong, VskDWordLong;
typedef float  VskSingle;
typedef double VskDouble;

// サイズを確かめる
static_assert(sizeof(VskChar)   == 1, "");
static_assert(sizeof(VskByte)   == 1, "");
static_assert(sizeof(VskShort)  == 2, "");
static_assert(sizeof(VskWord)   == 2, "");
static_assert(sizeof(VskLong)   == 4, "");
static_assert(sizeof(VskDword)  == 4, "");
static_assert(sizeof(VskSingle) == 4, "");
static_assert(sizeof(VskDouble) == 8, "");

// VskIntとVskFloatは切り替え可能
#ifdef INT_IS_32BIT
    typedef  std::int32_t VskInt;
#else
    typedef  std::int16_t VskInt;
#endif
#ifdef FLOAT_IS_DOUBLE
    typedef double VskFloat;
#else
    typedef float VskFloat;
#endif

// 型を表す文字
typedef VskByte VskType;
enum VSK_TYPE : uint8_t
{
    VSK_TYPE_SINGLE  = '!',
    VSK_TYPE_DOUBLE  = '#',
    VSK_TYPE_INTEGER = '%',
    VSK_TYPE_STRING  = '$',
    VSK_TYPE_LONG    = '&',
    VSK_TYPE_ARRAY   = '@',
};

typedef VskDouble VskLineNo;        // 行番号
typedef VskDouble VskColumnNo;      // 桁番号

typedef std::string    VskString;   // ANSI文字列
typedef std::wstring   VskWString;  // ワイド文字列
#ifdef UNICODE
    typedef VskWString VskTString;  // テキスト文字列
#else
    typedef VskString  VskTString;  // テキスト文字列
#endif

#define VSK_MAX_STR_LEN 255         // 文字列の長さの限界（無制限モードを除く）

typedef std::size_t     VskAddr;     // アドレス（符号なし）
typedef std::ptrdiff_t  VskPtrDiff;  // ポインタの差（符号付き）
typedef std::size_t     VskMemSize;  // メモリーサイズ（符号なし）

// 値の小数点以下を四捨五入して丸める
inline VskShort vsk_round(VskShort value)
{
    return value;
}
inline VskLong vsk_round(VskLong value)
{
    return value;
}
inline VskInt vsk_round(VskSingle value)
{
    return VskInt(std::round(value));
}
inline VskInt vsk_round(VskDouble value)
{
    return VskInt(std::round(value));
}

inline VskSingle vsk_sinf(VskSingle x)
{
#ifdef HAS_STD_SINF
    return std::sinf(x);
#else
    return VskSingle(std::sin(x));
#endif
}
inline VskSingle vsk_cosf(VskSingle x)
{
#ifdef HAS_STD_COSF
    return std::cosf(x);
#else
    return VskSingle(std::cos(x));
#endif
}

//////////////////////////////////////////////////////////////////////////////
// 正常性チェック

#if defined(UNICODE) != defined(_UNICODE)
    #error
#endif

//////////////////////////////////////////////////////////////////////////////
// _countof

#ifndef _countof
    #define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

//////////////////////////////////////////////////////////////////////////////
// 抽象オブジェクト クラス

struct VskObject
{
    static long s_cAliveObjects; // オブジェクトの生存数を管理する

    VskObject();
    virtual ~VskObject();
};

//////////////////////////////////////////////////////////////////////////////
// インデックスリスト

typedef size_t VskIndex;
typedef std::deque<VskIndex> VskIndexList;

//////////////////////////////////////////////////////////////////////////////
// VskPoint, VskSize, VskRect

// VskPoint - (x, y)座標
template <typename T_VALUE>
struct VskPoint
{
    T_VALUE m_x, m_y;
};
typedef VskPoint<int> VskPointI;
typedef VskPoint<VskSingle> VskPointS;

inline VskPointI vsk_round(const VskPointS& pt)
{
    return { vsk_round(pt.m_x), vsk_round(pt.m_y) };
}

// VskSize - サイズ
template <typename T_VALUE>
struct VskSize
{
    T_VALUE m_cx, m_cy;
};
typedef VskSize<int> VskSizeI;
typedef VskSize<VskSingle> VskSizeS;

inline VskSizeI vsk_round(const VskSizeS& siz)
{
    return { vsk_round(siz.m_cx), vsk_round(siz.m_cy) };
}

// VskRect - 長方形領域の座標
template <typename T_VALUE>
union VskRect
{
    struct
    {
        T_VALUE m_x0, m_y0, m_x1, m_y1;
    };
    struct
    {
        VskPoint<T_VALUE> m_pt0, m_pt1;
    };
    enum { m_is_integer = std::numeric_limits<T_VALUE>::is_integer };

    VskRect()
    {
        set_empty();
    }
    VskRect(const VskPoint<T_VALUE>& pt0, const VskPoint<T_VALUE>& pt1)
    {
        m_pt0 = pt0;
        m_pt1 = pt1;
    }
    VskRect(T_VALUE x0, T_VALUE y0, T_VALUE x1, T_VALUE y1)
    {
        m_x0 = x0; m_y0 = y0;
        m_x1 = x1; m_y1 = y1;
    }

    bool empty() const
    {
        return m_x0 >= m_x1 + m_is_integer || m_y0 >= m_y1 + m_is_integer;
    }
    void set_empty()
    {
        m_x0 = m_y0 = 0;
        m_x1 = m_y1 = -m_is_integer;
    }
    bool inside_x(const T_VALUE& x) const
    {
        return m_x0 <= x && x <= m_x1;
    }
    bool inside_y(const T_VALUE& y) const
    {
        return m_y0 <= y && y <= m_y1;
    }
    bool inside(const T_VALUE& x, const T_VALUE& y) const
    {
        return inside_x(x) && inside_y(y);
    }
    bool inside(const VskPoint<T_VALUE>& pt) const
    {
        return inside(pt.m_x0, pt.m_y0);
    }
    T_VALUE width() const
    {
        if (empty()) return 0;
        return m_x1 - m_x0;
    }
    T_VALUE height() const
    {
        if (empty()) return 0;
        return m_y1 - m_y0;
    }
    VskPoint<T_VALUE> center_point() const
    {
        return { (m_x0 + m_x1) / 2, (m_y0 + m_y1) / 2 };
    }

    bool intersect(const VskRect<T_VALUE>& other);
};
typedef VskRect<int> VskRectI;
typedef VskRect<VskSingle> VskRectS;

inline VskRectI vsk_round(const VskRectS& rc)
{
    return { vsk_round(rc.m_x0), vsk_round(rc.m_y0), vsk_round(rc.m_x1), vsk_round(rc.m_y1) };
}

//////////////////////////////////////////////////////////////////////////////

// 1セクタは256バイト
#define VSK_SECTOR_SIZE 256

//////////////////////////////////////////////////////////////////////////////
