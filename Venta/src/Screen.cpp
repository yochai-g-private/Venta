#if !_USE_MEGA_ESP
#include "Screen.h"
#include "Shaon.h"
#include "Screen.h"
#include <IInput.h>
#include <Hysteresis.h>
#include <LiquidCrystalDisplay.h>
#include <TimeEx.h>

#define LCD_EXISTS      1

#if LCD_EXISTS

static bool lcd_OK = false;
#define CHECK_LCD_OK        if(!lcd_OK)     return

static LiquidCrystalDisplay*    pLcd;
#define lcd                     (*pLcd)

static AnalogInputPin           ___potentiometer(POTENTIOMETER_PIN);
static DivisorHysteresis        backlight_level(___potentiometer, 11);
#endif //LCD_EXISTS
//-----------------------------------------------------------------
void Screen::Show(const char* s1, const char* s2)
{
#if LCD_EXISTS
    CHECK_LCD_OK;

    lcd->clear();
	lcd->setCursor(0, 0);
	lcd->print(s1);
	lcd->setCursor(0, 1);
	lcd->print(s2);
#endif //LCD_EXISTS
}
//-----------------------------------------------------------------
void Screen::Show(const char* s)
{
#if LCD_EXISTS
    CHECK_LCD_OK;

    lcd->clear();
	lcd->setCursor(0, 0);
	lcd->print(s);
#endif //LCD_EXISTS
}
//-----------------------------------------------------------------
#if _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
void Screen::ShowHeb(const char* s, uint8_t row, bool clear)
{
#if LCD_EXISTS
    CHECK_LCD_OK;

     if(clear)
        lcd->clear();

     lcd.PrintHeb(s, row);
#endif //LCD_EXISTS
}
#endif //_USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
//-----------------------------------------------------------------
void Screen::ShowLogo()
{
#if LCD_EXISTS
    CHECK_LCD_OK;

#if _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
    const char* sss = " שעון שבת שלום!";
    ShowHeb(sss, 0, true);
    lcd->setCursor(0, 1);
	lcd->print("*** GLAUBERS ***");    
#else
     Show("* Shaon Shomer *",
         "*    SHABAT    *");
#endif// _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
#endif //LCD_EXISTS
 }
//-----------------------------------------------------------------
void Screen::OnLoop()
{
#if LCD_EXISTS
    CHECK_LCD_OK;

#if 1
    return;
#else
    static int st_backlight_level = -1;

    int curr_backlight_level = backlight_level.Get();

    if(curr_backlight_level > 100)        curr_backlight_level = 100;
    else if(curr_backlight_level < 0)     curr_backlight_level = 0;

    if(st_backlight_level == curr_backlight_level)
        return;

    st_backlight_level = curr_backlight_level;

    lcd->setBacklight(st_backlight_level);
//#else
    static int prev_second = -1;
    int second = FixTime::Now().GetSeconds() / 10;

    if(prev_second == second)
        return;

    prev_second = second;

    static int st_backlight_level = 100;
    LOGGER << st_backlight_level << NL;
    lcd->setBacklight(st_backlight_level);
    st_backlight_level -= 10;
    if(st_backlight_level < 0)  st_backlight_level = 100;
#endif
#endif //LCD_EXISTS
}
//-----------------------------------------------------------------
void Screen::Initialize()
{
#if LCD_EXISTS

    LCD::CreateContext create_context(LCD_2x16_PARAMS);

    Wire.begin();
    Wire.beginTransmission(create_context.I2C_Address);
    byte error = Wire.endTransmission();

    if(error)
    {
        LOGGER << "LCD not found!" << NL;
        return;
    }

    lcd_OK = true;

    static LiquidCrystalDisplay	    st_lcd(create_context);
    pLcd = &st_lcd;

    OnLoop();

    ShowLogo();
#else
    LOGGER << "LCD does not exist!" << NL;
#endif //LCD_EXISTS
}
//-----------------------------------------------------------------
#endif //!_USE_MEGA_ESP
