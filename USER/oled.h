#include "n32g430.h"



#define MOC_Data		0x40
#define MOC_Command	0x00 

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, etc.).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.
    SSD1306_128_64  128x64 pixel display
    -----------------------------------------------------------------------*/
#define SSD1306_128_64
/*=========================================================================*/
void oled_init(void);	//call this function once
void Out_Oled(uint8_t cmd,uint8_t data);
void OLED_Clear(void)  ;
void OLED_Refresh_Gram(void);
void OLED_SETBrightness(uint8_t value);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void OLED_ShowCNString(u8 x, u8 y, const u8 *p, u8 size, u8 num);
void OLED_ShowNum(u8 x, u8 y, int num, u8 len, u8 size);
void OLED_ShowFloat(u8 x, u8 y, double num, u8 precisefloat, u8 size);
void OLED_ShowUnFloat(u8 x, u8 y, double num, u8 precisenum, u8 precisefloat, u8 size);
void OLED_DrawLine( unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2,unsigned char mode ) ;
#define  add   			1
#define  erase   		2
#define  invert   		3


#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 32
#endif
#if defined SSD1306_96_16
  #define SSD1306_LCDWIDTH                  96
  #define SSD1306_LCDHEIGHT                 16
#endif

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

