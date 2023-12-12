#ifndef __XMBMP_GRAPHICS_H__
#define __XMBMP_GRAPHICS_H__
#include <NoRSX.h>
#include <string>

#define MENU_ROLL_OPTIONS 5
#define SUBMENU_ROLL_OPTIONS 7

using namespace std;

extern NoRSX *Graphics;
extern Bitmap BMap;
extern NoRSX_Bitmap Menu_Layer;
extern Background B1;
extern Font F1, F2;
extern Image IBackground, ILogo, IBCross, IBCircle, IBSquare, IBTriangle, IBUp, IBDown, ISDown, ISUp, ICalendar;
extern pngData png_logo, png_background, png_button_cross, png_button_circle, png_button_square, png_button_triangle, png_button_up, png_button_down, png_scroll_up, png_scroll_down, png_calendar;
extern MsgDialog Mess;
extern Printf PF;

extern msgType MSG_OK, MSG_ERROR, MSG_YESNO_DNO, MSG_YESNO_DYES;

int ypos(int y);
int xpos(int x);
s32 center_text_x(int fsize, const char* message);
u32 menu_text_color(int current, int selected, int choosed, int backups, int menu1_restore);
void bitmap_inititalize(string screensize, string folder);
void bitmap_background(string jailbreak, string version, string type, string currentDateTime);
void draw_menu(int choosed);

#endif
