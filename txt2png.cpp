// License: MIT
#include <windows.h>
#include <windowsx.h>

// イメージハンドル
typedef HBITMAP VskImageHandle;

#include "txt2png.h"
#include "encoding.h"

void version(void)
{
    std::puts("txt2png by katahiromz Version 0.8");
}

void usage(void)
{
    std::printf(
        "Usage: txt2png [options] -i your_list.bas\n"
        "\n"
        "Options:\n"
        "    --help                Display this message\n"
        "    --version             Show version information\n"
        "    -i INPUT              Specify input file (program list or text)\n"
        "    --max-x COLUMNS       Specify column count (default: 120)\n"
        "    --max-y ROWS          Specify row count (default: 80)\n"
        "    --margin MARGIN       Specify margin in pixels (default: 16)\n"
        "    --8801                Use 8801 font\n"
        "    --bold                Use bold font\n"
        "\n"
        "Output files will be output-1.png, output-2.png etc.\n"
    );
}

typedef COLORREF VskSystemColor;

// １バイト中のビット群を逆順にしたものを返す関数
VskByte vsk_reverse_byte(VskByte x)
{
    static const VskByte table[] =
    {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };
    return table[x];
}

// SJIS全角文字をJIS全角文字に変換
VskWord vsk_sjis2jis(VskByte high, VskByte low)
{
    high <<= 1;
    if (low < 0x9F)
    {
        if (high < 0x3F)
            high += 0x1F;
        else
            high -= 0x61;
        if (low > 0x7E)
            low -= 0x20;
        else
            low -= 0x1F;
    }
    else
    {
        if (high < 0x3F)
            high += 0x20;
        else
            high -= 0x60;
        low -= 0x7E;
    }
    return ((VskWord(high) << 8) | low);
}

// XBMからビットの深さが1BPPのDIBビットマップを作成
VskImageHandle vsk_create_1bpp_image_from_xbm(int width, int height, const void *bits)
{
    assert(width % CHAR_BIT == 0);
    size_t size = width / CHAR_BIT * height;
    std::vector<VskByte> bytes;
    bytes.resize(size);
    memcpy(bytes.data(), bits, size);
    for (auto& byte : bytes)
        byte = vsk_reverse_byte(byte);
    return CreateBitmap(width, height, 1, 1, bytes.data());
}

// ビットの深さが32BPPのDIBビットマップを作成
VskImageHandle vsk_create_32bpp_image(int width, int height, void **ppvBits)
{
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    HDC hDC = CreateCompatibleDC(nullptr);
    LPVOID pvBits;
    HBITMAP hbm = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    if (ppvBits)
        *ppvBits = pvBits;
    DeleteDC(hDC);
    return hbm;
}

// ビットマップをクリップボードにコピーする
bool vsk_copy_image_to_clipboard(HWND hwnd, HBITMAP hBitmap)
{
    BITMAP bm;
    if (!GetObject(hBitmap, sizeof(bm), &bm))
    {
        assert(0);
        return false;
    }

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = bm.bmWidth;
    bmi.bmiHeader.biHeight = bm.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;

    SIZE_T cb = sizeof(BITMAPINFOHEADER) + bm.bmWidthBytes * bm.bmHeight;
    HANDLE hDIB = GlobalAlloc(GHND | GMEM_SHARE, cb);
    if (!hDIB)
    {
        assert(0);
        return false;
    }

    BYTE *pb = (BYTE *)GlobalLock(hDIB);
    if (!pb)
    {
        assert(0);
        GlobalFree(hDIB);
        return false;
    }

    CopyMemory(pb, &bmi, sizeof(BITMAPINFOHEADER));

    HDC hDC = CreateCompatibleDC(NULL);
    GetDIBits(hDC, hBitmap, 0, bm.bmHeight, pb + sizeof(BITMAPINFOHEADER), &bmi, DIB_RGB_COLORS);
    DeleteDC(hDC);

    GlobalUnlock(hDIB);

    BOOL bOK = FALSE;
    if (OpenClipboard(hwnd))
    {
        if (EmptyClipboard())
        {
            bOK = (SetClipboardData(CF_DIB, hDIB) != NULL);
            assert(bOK);
        }
        else
        {
            assert(0);
        }
        CloseClipboard();
    }
    else
    {
        assert(0);
    }

    if (!bOK)
        GlobalFree(hDIB);

    return !!bOK;
}

////////////////////////////////////////////////////////////////////////////////////
// VskImage

// VskImageクラスの実装
struct VskImageImpl;

// イメージを扱うクラス
struct VskImage
{
    struct VskImageImpl *m_pimpl;

    operator VskImageHandle() const;

    VskImage();
    VskImage(VskImageHandle image);
    ~VskImage();

    int width() const;
    int height() const;
    int bpp() const;
    int pitch() const;

    void attach(VskImageHandle image);
    VskImageHandle detach();

    void destroy();

    VskByte *bits(size_t offset = 0);
    const VskByte *bits(size_t offset = 0) const;

    virtual VskSystemColor get_pixel(int x, int y) const { return 0; }
    virtual void set_pixel(int x, int y, VskSystemColor color) { }

    bool inside(int x, int y) const;
};

// 1BPPのイメージを扱うクラス
struct Vsk1BppImage : VskImage
{
    Vsk1BppImage();
    Vsk1BppImage(VskImageHandle image);
    Vsk1BppImage(int width, int height, const void *bits);
};

// VskImageの内部実装VskImageImpl
struct VskImageImpl
{
    VskImageHandle m_image = nullptr;
    BITMAP m_bm = { 0 };
};

// イメージのハンドル
VskImage::operator VskImageHandle() const
{
    return m_pimpl->m_image;
}

// コンストラクタ
VskImage::VskImage()
{
    m_pimpl = new VskImageImpl;
}

// コンストラクタ
VskImage::VskImage(VskImageHandle image)
{
    m_pimpl = new VskImageImpl;
    attach(image);
}

// デストラクタ
VskImage::~VskImage()
{
    destroy();
    delete m_pimpl;
}

// 幅（ピクセル単位）
int VskImage::width() const
{
    return m_pimpl->m_bm.bmWidth;
}

// 高さ（ピクセル単位）
int VskImage::height() const
{
    return m_pimpl->m_bm.bmHeight;
}

// ビットの深さ
int VskImage::bpp() const
{
    return m_pimpl->m_bm.bmBitsPixel;
}

// イメージの横幅（バイト数）
int VskImage::pitch() const
{
    return m_pimpl->m_bm.bmWidthBytes;
}

// イメージハンドルを取り付ける
void VskImage::attach(VskImageHandle image)
{
    destroy();
    if (GetObject(image, sizeof(m_pimpl->m_bm), &m_pimpl->m_bm))
        m_pimpl->m_image = image;
}

// イメージハンドルを取りはずす
VskImageHandle VskImage::detach()
{
    VskImageHandle old_image = m_pimpl->m_image;
    m_pimpl->m_image = nullptr;
    return old_image;
}

// イメージハンドルを破棄
void VskImage::destroy()
{
    if (m_pimpl->m_image)
    {
        DeleteObject(m_pimpl->m_image);
        m_pimpl->m_image = nullptr;
    }
}

// イメージのデータを取得する
VskByte *VskImage::bits(size_t offset)
{
    assert(m_pimpl->m_image);
    assert(m_pimpl->m_bm.bmBits);
    return reinterpret_cast<VskByte *>(m_pimpl->m_bm.bmBits) + offset;
}

// イメージのデータを取得する
const VskByte *VskImage::bits(size_t offset) const
{
    assert(m_pimpl->m_image);
    assert(m_pimpl->m_bm.bmBits);
    return reinterpret_cast<VskByte *>(m_pimpl->m_bm.bmBits) + offset;
}

// 座標(x, y)はイメージの内側か？
bool VskImage::inside(int x, int y) const
{
    return x >= 0 && y >= 0 && x < width() && y < height();
}

////////////////////////////////////////////////////////////////////////////////////
// Vsk1BppImage

// コンストラクタ
Vsk1BppImage::Vsk1BppImage()
{
}

// コンストラクタ
Vsk1BppImage::Vsk1BppImage(VskImageHandle image) : VskImage(image)
{
}

// コンストラクタ
Vsk1BppImage::Vsk1BppImage(int width, int height, const void *bits)
{
    attach(vsk_create_1bpp_image_from_xbm(width, height, bits));
}

////////////////////////////////////////////////////////////////////////////////////

// ANK文字のピクセルを取得するクラス
template <int t_width = 128, int t_height = 256>
struct VskAnkGetter {
    const VskByte *m_bits = nullptr;
    VskAnkGetter() = default;
    constexpr VskAnkGetter(int width, int height, const VskByte *bits) : m_bits(bits) {
        assert(t_width == width);
        assert(t_height == height);
    }
    constexpr void set(int width, int height, const VskByte *bits) {
        m_bits = bits;
        assert(t_width == width);
        assert(t_height == height);
    }
    bool operator()(int x, int y) const {
        if (0 <= x && x < t_width && 0 <= y && y < t_height) {
            int offset = y * (t_width / CHAR_BIT) + x / CHAR_BIT;
            auto& byte = m_bits[offset];
            return ((byte >> (x % CHAR_BIT)) & 1);
        }
        return false;
    }
};

// 8801っぽいANKのピクセルを取得するクラス
struct Vsk8801AnkGetter : VskAnkGetter<> {
    Vsk8801AnkGetter() {
#include "img/pc88_chars.xbm"
        set(pc88_chars_width, pc88_chars_height, pc88_chars_bits);
    }
};

// 9801っぽいANKのピクセルを取得するクラス
struct Vsk9801AnkGetter : VskAnkGetter<> {
    Vsk9801AnkGetter() {
#include "img/pc98_chars.xbm"
        set(pc98_chars_width, pc98_chars_height, pc98_chars_bits);
    }
};

// 全角文字のピクセルを取得するクラス
struct VskKanjiGetter {
    const VskByte *m_bits = nullptr;
    VskKanjiGetter() {
#include "img/kanji_chars.xbm"
        m_bits = kanji_chars_bits;
    }
    enum {
        t_width = kanji_chars_width,
        t_height = kanji_chars_height,
    };
    bool operator()(int x, int y) const {
        if (0 <= x && x < t_width && 0 <= y && y < t_height) {
            int offset = y * (t_width / CHAR_BIT) + x / CHAR_BIT;
            auto& byte = m_bits[offset];
            return ((byte >> (x % CHAR_BIT)) & 1);
        }
        return false;
    }
};

// ピクセルを置かないクラス
struct VskNullPutter
{
    void operator()(int x, int y) { }
};

// ANK文字を描画する
template <typename T_PUTTER, typename T_ERASER, typename T_GETTER>
inline void vk_draw_ank(T_PUTTER& putter, T_ERASER& eraser, int x0, int y0, VskByte ch, const T_GETTER& getter, bool underline, bool upperline = false)
{
    auto xSrc = (ch & 0xF) * 8, ySrc = (ch >> 4) * 16;
    for (int dy = 0, y = y0; dy < 16; ++y, ++dy) {
        bool flag = (upperline && dy == 0) || (underline && dy == 15);
        for (int dx = 0, x = x0; dx < 8; ++x, ++dx) {
            if (getter(xSrc + dx, ySrc + dy) | flag)
                putter(x, y);
            else
                eraser(x, y);
        }
    }
}

// JISの全角文字を描画する(共通処理)
template <typename T_GETTER, typename T_PUTTER, typename T_ERASER>
inline void vk_draw_jis_generic(T_GETTER& getter, T_PUTTER& putter, T_ERASER& eraser, int x0, int y0, int x1, int y1, int xSrc, int ySrc, VskWord jis, bool underline, bool upperline = false)
{
    for (int dy = 0, y = y0; dy < 16; ++y, ++dy) {
        bool flag = (upperline && dy == 0) || (underline && dy == 15);
        for (int dx = 0, x = x0; dx < 8; ++x, ++dx) {
            if (getter(xSrc + dx, ySrc + dy) | flag)
                putter(x, y);
            else
                eraser(x, y);
        }
    }
    for (int dy = 0, y = y1; dy < 16; ++y, ++dy) {
        bool flag = (upperline && dy == 0) || (underline && dy == 15);
        for (int dx = 8, x = x1; dx < 16; ++x, ++dx) {
            if (getter(xSrc + dx, ySrc + dy) | flag)
                putter(x, y);
            else
                eraser(x, y);
        }
    }
}

// JISの全角文字を描画する
template <typename T_PUTTER, typename T_ERASER>
inline void vk_draw_jis(T_PUTTER& putter, T_ERASER& eraser, int x0, int y0, int x1, int y1, VskWord jis, bool underline, bool upperline = false)
{
    VskKanjiGetter getter;
    auto xSrc = (vsk_low_byte(jis) - 0x21) * 16, ySrc = (vsk_high_byte(jis) - 0x21) * 16;
    vk_draw_jis_generic(getter, putter, eraser, x0, y0, x1, y1, xSrc, ySrc, jis, underline, upperline);
}

////////////////////////////////////////////////////////////////////////////////////

bool vsk_text_to_bitmap(VskTextToPng& text2png)
{
    std::string& text = text2png.m_text;
    if (text.empty())
        return false;

    int max_x = text2png.m_max_x, max_y = text2png.m_max_y;
    int margin = text2png.m_margin;
    int page = text2png.m_page;
    bool is_8801 = text2png.m_is_8801, bold = text2png.m_bold;

    VskImageHandle& hbm = text2png.m_hbm;
    hbm = nullptr;

    const int char_width = (bold ? 9 : 8), char_height = 20;
    int cx = char_width*max_x + 2*margin, cy = char_height*max_y + 2*margin;

    HDC hDC;
    HGDIOBJ hbmOld;
    if (page > 0)
    {
        hbm = (HBITMAP)vsk_create_32bpp_image(cx, cy, nullptr);
        if (!hbm)
            return false;

        hDC = CreateCompatibleDC(NULL);
        hbmOld = SelectObject(hDC, hbm);
        RECT rc = { 0, 0, cx, cy };
        FillRect(hDC, &rc, GetStockBrush(WHITE_BRUSH));
    }

    VskNullPutter null_putter;
    auto black_putter = [&](int x, int y) {
        if (bold) {
            SetPixel(hDC, x, y, RGB(0, 0, 0));
            SetPixel(hDC, x + 1, y, RGB(0, 0, 0));
        } else {
            SetPixel(hDC, x, y, RGB(0, 0, 0));
        }
    };

    Vsk8801AnkGetter getter88;
    Vsk9801AnkGetter getter98;
    auto getter = [&](int x, int y) {
        if (is_8801)
            return getter88(x, y);
        return getter98(x, y);
    };

    int current_page = 1;
    int x = 0, y = 0;
    bool was_lead = false;
    char lead = 0;
    for (auto& ch : text)
    {
        if (x >= max_x)
        {
            x = 0;
            ++y;
        }
        if (was_lead)
        {
            was_lead = false;
            int x0 = margin + char_width*(x - 1), y0 = margin + char_height*y;
            if (vsk_is_sjis_trail(ch))
            {
                VskWord jis = vsk_sjis2jis(lead, ch);
                if (page == current_page)
                    vk_draw_jis(black_putter, null_putter, x0, y0, x0 + 8, y0, jis, false, false);
                ++x;
                continue;
            }
            else
            {
                if (page == current_page)
                    vk_draw_ank(black_putter, null_putter, x0, y0, lead, getter, false, false);
            }
        }
        if (ch == '\r')
            continue;
        if (ch == '\n')
        {
            x = 0;
            ++y;
            if (y >= max_y)
            {
                x = y = 0;
                if (current_page == page && page > 0)
                    break;
                ++current_page;
            }
            continue;
        }

        if (vsk_is_sjis_lead(ch))
        {
            lead = ch;
            was_lead = true;
            ++x;
            continue;
        }

        if (page == current_page)
        {
            int x0 = margin + char_width*x, y0 = margin + char_height*y;
            vk_draw_ank(black_putter, null_putter, x0, y0, ch, getter, false, false);
        }
        ++x;
    }

    if (page <= 0)
    {
        text2png.m_total_pages = current_page;
        return true;
    }

    SelectObject(hDC, hbmOld);
    DeleteDC(hDC);
    return true;
}

#ifdef TXT2PNG_EXE

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// GDI+用。エンコーダーのCLSIDを取得する
BOOL GetEncoderClsid(CLSID* pClsid, LPCWSTR mime_type)
{
    // エンコーダーのサイズ情報を取得
    UINT cItems = 0, cbEncoders = 0;
    Gdiplus::GetImageEncodersSize(&cItems, &cbEncoders);
    if (!cbEncoders)
        return FALSE; // 失敗

    // エンコーダーを格納するメモリーを確保
    auto pEncoders = (Gdiplus::ImageCodecInfo *)malloc(cbEncoders);
    if (!pEncoders)
        return FALSE; // 失敗

    // エンコーダーを格納
    Gdiplus::GetImageEncoders(cItems, cbEncoders, pEncoders);

    // MIMEの種類に一致するエンコーダーのCLSIDを取得
    for (UINT iItem = 0; iItem < cItems; ++iItem)
    {
        if (wcscmp(pEncoders[iItem].MimeType, mime_type) == 0)
        {
            *pClsid = pEncoders[iItem].Clsid;
            free(pEncoders); // 解放
            return TRUE; // 成功
        }
    }

    free(pEncoders); // 解放
    return FALSE; // 失敗
}

// HBITMAPを画像ファイルとして保存する関数
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCWSTR filename, LPCWSTR mime_type = L"image/png")
{
    // GDI+ BitmapオブジェクトをHBITMAPから作成
    auto bitmap = new Gdiplus::Bitmap(hBitmap, NULL);
    if (!bitmap)
        return FALSE; // 失敗
    std::shared_ptr<Gdiplus::Bitmap> pBitmap(bitmap);

    // PNGエンコーダのCLSIDを取得
    CLSID clsid;
    if (!GetEncoderClsid(&clsid, mime_type))
        return FALSE; // 失敗

    // PNGとして保存
    if (pBitmap->Save(filename, &clsid, NULL) != Gdiplus::Ok)
        return FALSE; // 失敗

    return TRUE; // 成功
}

// スクリーンショットを保存する
bool vsk_save_screenshot(HBITMAP hbm, const char *out_filename)
{
    // GDI+を初期化
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WCHAR szFileW[MAX_PATH];
    ::MultiByteToWideChar(932, 0, out_filename, -1, szFileW, MAX_PATH);
    szFileW[MAX_PATH - 1] = 0;

    // 画像ファイルとして保存
    SaveHBITMAPToFile(hbm, szFileW, L"image/png");

    // GDI+を解放
    Gdiplus::GdiplusShutdown(gdiplusToken);

    // HBITMAPを破棄
    ::DeleteObject(hbm);

    return true;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return 0;
    }

    std::string input;
    int margin = 16, max_x = 120, max_y = 80;
    bool is_8801 = false;
    bool bold = false;
    for (int iarg = 1; iarg < argc; ++iarg)
    {
        std::string arg = argv[iarg];
        if (arg == "--help")
        {
            usage();
            return 0;
        }
        if (arg == "--version")
        {
            version();
            return 0;
        }
        if (arg == "--margin")
        {
            if (++iarg < argc)
            {
                margin = atoi(argv[iarg]);
            }
            continue;
        }
        if (arg == "--max-x")
        {
            if (++iarg < argc)
            {
                max_x = atoi(argv[iarg]);
            }
            continue;
        }
        if (arg == "--max-y")
        {
            if (++iarg < argc)
            {
                max_y = atoi(argv[iarg]);
            }
            continue;
        }
        if (arg == "--8801")
        {
            is_8801 = true;
            continue;
        }
        if (arg == "--bold")
        {
            bold = true;
            continue;
        }
        if (arg == "-i")
        {
            if (++iarg < argc)
            {
                input = argv[iarg];
            }
            continue;
        }
        fprintf(stderr, "LINE2PNG: Invalid argument '%s'\n", arg.c_str());
        return 1;
    }

    if (input.empty())
    {
        fprintf(stderr, "LINE2PNG: No input file specified\n");
        return 1;
    }

    FILE *fin = fopen(input.c_str(), "rb");
    char buf[256];
    std::string text;
    while (fgets(buf, 256, fin))
    {
        text += buf;
    }
    fclose(fin);

    VskTextToPng text2png;
    text2png.m_text = text;
    text2png.m_max_x = max_x;
    text2png.m_max_y = max_y;
    text2png.m_margin = margin;
    text2png.m_page = 0;
    text2png.m_is_8801 = is_8801;
    text2png.m_bold = bold;
    vsk_text_to_bitmap(text2png);

    int num_pages = text2png.m_total_pages;
    for (int ipage = 1; ipage <= num_pages; ++ipage)
    {
        char out_filename[MAX_PATH];
        std::sprintf(out_filename, "output-%u.png", ipage);

        text2png.m_page = ipage;
        vsk_text_to_bitmap(text2png);
        vsk_save_screenshot(text2png.m_hbm, out_filename);
        DeleteObject(text2png.m_hbm);

        printf("Generated %s.\n", out_filename);
    }

    printf("Total %d pages\n", num_pages);
    return 0;
}

#endif  // def TXT2PNG_EXE
