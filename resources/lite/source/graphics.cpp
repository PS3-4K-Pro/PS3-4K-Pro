#include "graphics.h"
#include "filesystem.h"

NoRSX *Graphics = new NoRSX();
Background B1(Graphics);
Bitmap BMap(Graphics);
NoRSX_Bitmap Menu_Layer;
Font F1(("/dev_hdd0/game/"+string(APP_TITLEID)+"/USRDIR/data/fonts/titles.ttf").c_str(), Graphics);
Font F2(("/dev_hdd0/game/"+string(APP_TITLEID)+"/USRDIR/data/fonts/items.ttf").c_str(), Graphics);

//Font F1(LATIN2, Graphics), F2(LATIN2, Graphics);
Image IBackground(Graphics), ILogo(Graphics), IBCross(Graphics), IBCircle(Graphics), IBSquare(Graphics), IBTriangle(Graphics), IBUp(Graphics), IBDown(Graphics), ISDown(Graphics), ISUp(Graphics), ICalendar(Graphics);
pngData png_logo, png_background, png_button_cross, png_button_circle, png_button_square, png_button_triangle, png_button_up, png_button_down, png_scroll_up, png_scroll_down, png_calendar;
MsgDialog Mess(Graphics);
Printf PF(("/dev_hdd0/game/"+(string)APP_TITLEID+"/USRDIR/data/log.txt").c_str());

msgType MSG_OK = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON); //| MSG_DIALOG_BKG_INVISIBLE
msgType MSG_ERROR = (msgType)(MSG_DIALOG_ERROR | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON); //| MSG_DIALOG_BKG_INVISIBLE
msgType MSG_YESNO_DNO = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO | MSG_DIALOG_DEFAULT_CURSOR_NO); //| MSG_DIALOG_BKG_INVISIBLE
msgType MSG_YESNO_DYES = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO); //| MSG_DIALOG_BKG_INVISIBLE

int ypos(int y)
{
	return (int)(y*Graphics->height)/720;
}

int xpos(int x)
{
	return (int)(x*Graphics->width)/1280;
}

s32 center_text_x(int fsize, const char* message)
{
	return (Graphics->width-(strlen(message)*fsize/2))/2;
}

u32 menu_text_color(int current, int selected, int choosed, int backups, int menu1_restore)
{
	if (backups==0 && menu1_restore!=0) return COLOR_GREY;
	else if (current==selected)
	{
		if (choosed==1) return 0xff6500;
		else return COLOR_YELLOW;
	}
	else return COLOR_WHITE;
}

void bitmap_inititalize(string screensize, string folder)
{
	BMap.GenerateBitmap(&Menu_Layer);
    IBackground.LoadPNG((folder+"/data/images/"+screensize+"/background.png").c_str(), &png_background);
    ILogo.LoadPNG((folder+"/data/images/"+screensize+"/logo.png").c_str(), &png_logo);
    IBCross.LoadPNG((folder+"/data/images/"+screensize+"/cross.png").c_str(), &png_button_cross);
    IBCircle.LoadPNG((folder+"/data/images/"+screensize+"/circle.png").c_str(), &png_button_circle);
    IBSquare.LoadPNG((folder+"/data/images/"+screensize+"/square.png").c_str(), &png_button_square);
    IBTriangle.LoadPNG((folder+"/data/images/"+screensize+"/triangle.png").c_str(), &png_button_triangle);
	IBUp.LoadPNG((folder+"/data/images/"+screensize+"/up.png").c_str(), &png_button_up);
    IBDown.LoadPNG((folder+"/data/images/"+screensize+"/down.png").c_str(), &png_button_down);
    ISUp.LoadPNG((folder+"/data/images/"+screensize+"/scrollup.png").c_str(), &png_scroll_up);
    ISDown.LoadPNG((folder+"/data/images/"+screensize+"/scrolldown.png").c_str(), &png_scroll_down);
	ICalendar.LoadPNG((folder+"/data/images/"+screensize+"/calendar.png").c_str(), &png_calendar);
}

void bitmap_background(string jailbreak, string version, string type, string currentDateTime, string buildNumber)
{
	int sizeFont = ypos(30);

	B1.MonoBitmap(COLOR_BLACK,&Menu_Layer);
	IBackground.AlphaDrawIMGtoBitmap(0,0,&png_background,&Menu_Layer);
	u32 imgX =(Graphics->width/2)-(png_logo.width/2), imgY = ypos(30);
	ILogo.AlphaDrawIMGtoBitmap(imgX,imgY,&png_logo,&Menu_Layer);
	
	//Callendar
	ICalendar.AlphaDrawIMGtoBitmap(xpos(1050),ypos(42),&png_calendar,&Menu_Layer);
	int newSizeFont = sizeFont - 6; // Increase/Decrease default font size for this item
	F2.PrintfToBitmap(xpos(1050) + png_calendar.width + xpos(10), ypos(45) + newSizeFont - ypos(5), &Menu_Layer, COLOR_WHITE, newSizeFont, "%s", currentDateTime.c_str());
	F2.PrintfToBitmap(xpos(1050) + png_calendar.width + xpos(10), ypos(85) + newSizeFont - ypos(5), &Menu_Layer, COLOR_WHITE, newSizeFont, "%s", buildNumber.c_str());

	F2.PrintfToBitmap(center_text_x(sizeFont, "XXXXXXX Firmware: X.XX (XXX)"),Graphics->height-(sizeFont+ypos(20)+(sizeFont-ypos(5))+ypos(10)),&Menu_Layer,0x00ff00,sizeFont, "%s: %s (%s)", jailbreak.c_str(), version.c_str(), type.c_str());
	F2.PrintfToBitmap(center_text_x(sizeFont-ypos(5), "_TheUltimatePlayStationExperience__"),Graphics->height-((sizeFont-ypos(5))+ypos(10)),&Menu_Layer,0xb40898,sizeFont-ypos(5), "The Ultimate PlayStation Experience");
}

void draw_menu(int choosed)
{
	BMap.DrawBitmap(&Menu_Layer);
	Graphics->Flip();
	if (choosed==1) usleep(50*1000);
	else usleep(1000);
}
