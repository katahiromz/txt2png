#pragma once

#include "types.h"

struct VskTextToPng
{
    int m_total_pages = 0;
    std::string m_text;
    int m_max_x = 120;
    int m_max_y = 80;
    int m_margin = 16;
    int m_page = 1;
    bool m_is_8801 = false;
    bool m_bold = false;
    VskImageHandle m_hbm = nullptr;
};

bool vsk_text_to_bitmap(VskTextToPng& text2png);
