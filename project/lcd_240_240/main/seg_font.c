#include <stdio.h>
#include <stdlib.h>
#include "lvgl/lvgl.h"

/*Store the glyph descriptions*/
static const lv_font_glyph_dsc_t seg_font_glyph_dsc[] = 
{
  {.w_px = 13,	.glyph_index = 0},	/*Unicode: U+0020 ( )*/
  {.w_px = 8,	.glyph_index = 624},	/*Unicode: U+002e (.)*/
  {.w_px = 30,	.glyph_index = 1008},	/*Unicode: U+0030 (0)*/
  {.w_px = 16,	.glyph_index = 2448},	/*Unicode: U+0031 (1)*/
  {.w_px = 30,	.glyph_index = 3216},	/*Unicode: U+0032 (2)*/
  {.w_px = 30,	.glyph_index = 4656},	/*Unicode: U+0033 (3)*/
  {.w_px = 30,	.glyph_index = 6096},	/*Unicode: U+0034 (4)*/
  {.w_px = 32,	.glyph_index = 7536},	/*Unicode: U+0035 (5)*/
  {.w_px = 30,	.glyph_index = 9072},	/*Unicode: U+0036 (6)*/
  {.w_px = 30,	.glyph_index = 10512},	/*Unicode: U+0037 (7)*/
  {.w_px = 30,	.glyph_index = 11952},	/*Unicode: U+0038 (8)*/
  {.w_px = 30,	.glyph_index = 13392},	/*Unicode: U+0039 (9)*/
  {.w_px = 7,	.glyph_index = 14832},	/*Unicode: U+003a (:)*/
  {.w_px = 30,	.glyph_index = 15168},	/*Unicode: U+0041 (A)*/
  {.w_px = 32,	.glyph_index = 16608},	/*Unicode: U+0045 (E)*/
  {.w_px = 30,	.glyph_index = 18144},	/*Unicode: U+0052 (R)*/
  {.w_px = 31,	.glyph_index = 19584},	/*Unicode: U+0056 (V)*/
  {.w_px = 32,	.glyph_index = 21072},	/*Unicode: U+0057 (W)*/
};

/*List of unicode characters*/
static const uint32_t seg_font_unicode_list[] = {
  32,	/*Unicode: U+0020 ( )*/
  46,	/*Unicode: U+002e (.)*/
  48,	/*Unicode: U+0030 (0)*/
  49,	/*Unicode: U+0031 (1)*/
  50,	/*Unicode: U+0032 (2)*/
  51,	/*Unicode: U+0033 (3)*/
  52,	/*Unicode: U+0034 (4)*/
  53,	/*Unicode: U+0035 (5)*/
  54,	/*Unicode: U+0036 (6)*/
  55,	/*Unicode: U+0037 (7)*/
  56,	/*Unicode: U+0038 (8)*/
  57,	/*Unicode: U+0039 (9)*/
  58,	/*Unicode: U+003a (:)*/
  65,	/*Unicode: U+0041 (A)*/
  69,	/*Unicode: U+0045 (E)*/
  82,	/*Unicode: U+0052 (R)*/
  86,	/*Unicode: U+0056 (V)*/
  87,	/*Unicode: U+0057 (W)*/
  0,    /*End indicator*/
};

#include "spi_flash.h"
#define SEG_FONT_BASE_ADDR 0x1a0000
uint8_t *seg_font_bitmap = NULL;

const uint8_t * seg_font_get_bitmap_sparse(const lv_font_t * font, uint32_t unicode_letter)
{
    /*Check the range*/
    if(unicode_letter < font->unicode_first || unicode_letter > font->unicode_last) return NULL;

    uint32_t i;
    size_t size;
    for(i = 0; font->unicode_list[i] != 0; i++) {
        if(font->unicode_list[i] == unicode_letter) {
            size = sizeof(uint8_t) * font->h_px * font->glyph_dsc[i].w_px;
            if (seg_font_bitmap) {
              free(seg_font_bitmap);
              seg_font_bitmap = NULL;
            }
            seg_font_bitmap = (uint8_t *) malloc(size);
            
            spi_flash_read(SEG_FONT_BASE_ADDR + font->glyph_dsc[i].glyph_index, (void *)seg_font_bitmap, size);
            return seg_font_bitmap;
        }
    }

    return NULL;
}

lv_font_t seg_font = 
{
    .unicode_first = 32,	/*First Unicode letter in this font*/
    .unicode_last = 126,	/*Last Unicode letter in this font*/
    .h_px = 48,				/*Font height in pixels*/
    .glyph_bitmap = (uint8_t *)"font_seg.bin",	/*Bitmap of glyphs*/
    .glyph_dsc = seg_font_glyph_dsc,		/*Description of glyphs*/
    .glyph_cnt = 18,			/*Number of glyphs in the font*/
    .unicode_list = seg_font_unicode_list,	/*List of unicode characters*/
    .get_bitmap = seg_font_get_bitmap_sparse,	/*Function pointer to get glyph's bitmap*/
    .get_width = lv_font_get_width_sparse,	/*Function pointer to get glyph's width*/
    .bpp = 8,				/*Bit per pixel*/
    .monospace = 32,		/*Fix width (0: if not used)*/
    .next_page = NULL,		/*Pointer to a font extension*/
};