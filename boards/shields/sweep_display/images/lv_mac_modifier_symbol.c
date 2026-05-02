/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --font /usr/share/fonts/TTF/DejaVuSans-Bold.ttf --range 0x005e,0x21e7,0x2318,0x2325 --format lvgl -o boards/shields/sweep_display/images/lv_mac_modifier_symbol.c
 ******************************************************************************/

#ifdef __has_include
#if __has_include("lvgl.h")
#ifndef LV_LVGL_H_INCLUDE_SIMPLE
#define LV_LVGL_H_INCLUDE_SIMPLE
#endif
#endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_MAC_MODIFIER_SYMBOL
#define LV_MAC_MODIFIER_SYMBOL 1
#endif

#if LV_MAC_MODIFIER_SYMBOL

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+005E "^" */
    0xc, 0x7, 0x83, 0xf1, 0x8c, 0xc1, 0x80,

    /* U+21E7 "⇧" */
    0x0, 0x18, 0x24, 0x66, 0xa5, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x24, 0x3c,

    /* U+2318 "⌘" */
    0x70, 0xe8, 0x91, 0x89, 0x18, 0x91, 0x7f, 0xe0,
    0x90, 0x9, 0x7, 0xfe, 0x89, 0x18, 0x91, 0x89,
    0x17, 0xe,

    /* U+2325 "⌥" */
    0xfc, 0x1f, 0xfe, 0x1f, 0x7, 0x0, 0x3, 0x80,
    0x1, 0xc0, 0x0, 0xe0, 0x0, 0x60, 0x0, 0x3f,
    0x0, 0x1f
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 215, .box_w = 10, .box_h = 5, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 7, .adv_w = 215, .box_w = 8, .box_h = 12, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 238, .box_w = 12, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 295, .box_w = 16, .box_h = 9, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x2189, 0x22ba, 0x22c7
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 94, .range_length = 8904, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 4, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_mac_modifier_symbol = {
#else
lv_font_t lv_mac_modifier_symbol = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_MAC_MODIFIER_SYMBOL*/
