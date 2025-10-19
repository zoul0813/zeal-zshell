#ifndef LIBANSI_H
#define LIBANSI_H

#include <stdint.h>
#include <zvb_hardware.h>

#define ANSI_BUF_MAX 32

#define ANSI_IGNORE 0x00
#define ANSI_PRINT  0x01
#define ANSI_SEND   0x02

#define COLOR(fg, bg)      ((uint8_t) ((bg << 4 & 0xF0) | (fg & 0xF)))
#define GET_COLOR()        zvb_peri_text_color
#define GET_COLOR_BG()     (zvb_peri_text_color >> 4 & 0x0F)
#define GET_COLOR_FG()     (zvb_peri_text_color & 0x0F)
#define SET_COLOR(c)       zvb_peri_text_color = c
#define SET_COLORS(fg, bg) zvb_peri_text_color = COLOR(fg, bg)

#define ANSI_GRAPHICS_BOLD      (1 << 0)
#define ANSI_GRAPHICS_DIM       (1 << 1)
#define ANSI_GRAPHICS_ITALIC    (1 << 2)
#define ANSI_GRAPHICS_UNDERLINE (1 << 3)
#define ANSI_GRAPHICS_BLINK     (1 << 4)
#define ANSI_GRAPHICS_REVERSE   (1 << 5)
#define ANSI_GRAPHICS_INVIS     (1 << 6)
#define ANSI_GRAPHICS_STRIKE    (1 << 7)

#define ANSI_NULL 0x00
#define ANSI_SOH  0x01
#define ANSI_STX  0x02
#define ANSI_ETX  0x03
#define ANSI_ENQ  0x04
#define ANSI_ACK  0x05
#define ANSI_BEL  0x06
#define ANSI_BS   0x08
#define ANSI_TAB  0x09
#define ANSI_LF   0x0A
#define ANSI_VT   0x0B
#define ANSI_FF   0x0C
#define ANSI_CR   0x0D
#define ANSI_SO   0x0E
#define ANSI_SI   0x0F
#define ANSI_DLE  0x10
#define ANSI_DC1  0x11
#define ANSI_DC2  0x12
#define ANSI_DC3  0x13
#define ANSI_DC4  0x14
#define ANSI_NAK  0x15
#define ANSI_SYN  0x16
#define ANSI_ETB  0x17
#define ANSI_CAN  0x18
#define ANSI_EM   0x19
#define ANSI_SUB  0x1A
#define ANSI_ESC  0x1B
#define ANSI_FS   0x1C
#define ANSI_GS   0x1D
#define ANSI_RS   0x1E
#define ANSI_US   0x1F

#define ANSI_CSI  '['

uint8_t ansi_init(void);
uint8_t ansi_deinit(void);
uint8_t ansi_parse(unsigned char c, char response[ANSI_BUF_MAX]);
uint8_t ansi_print(char *buffer);

#endif