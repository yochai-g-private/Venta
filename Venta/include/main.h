#pragma once

#if !_USE_MEGA_ESP

#include <Logger.h>

enum Pins
{
    SEVEN_SEGMENT_DISPLAY_CLK_PIN   = D4,
    SEVEN_SEGMENT_DISPLAY_DIO_PIN   = D5,

    BUTTON_PIN                      = D6,

    _1HZ_TICKER_PIN                 = D7,
    BUZZER_PIN                      = D2,

    LED_RED_PIN                     = D12,
    LED_GREEN_PIN                   = D11,
    LED_BLUE_PIN                    = D10,

    POTENTIOMETER_PIN               = A7,
};

void SignalBuzzer(uint16_t on_off_millis, uint8_t n_times);
void StoreCfg();

#if _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
    #define ShowScreenMessage(English, Hebrew)  Screen::ShowHeb(Hebrew, 0, true)

    #define TEXT_ZERICHA    "זריחה"
    #define TEXT_SHEKYIA    "שקיעה"
    #define TEXT_SUMMER     "שעון קיץ"
    #define TEXT_WINTER     "שעון חורף"
    #define TEXT_HADLAKA    "הדלקת נרות"
    #define TEXT_HAVDALA    "הבדלה"
    #define TEXT_RESTART    "אתחל מחדש"
#else
    #define ShowScreenMessage(English, Hebrew)  Screen::Show(English)

    #define TEXT_ZERICHA    "Sun rise"
    #define TEXT_SHEKYIA    "Sun set"
    #define TEXT_SUMMER     "Switch Summer"
    #define TEXT_WINTER     "Switch Winter"
    #define TEXT_HADLAKA    "Hadlaka"
    #define TEXT_HAVDALA    "Havdala"
    #define TEXT_RESTART    "Restart"
#endif //_USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay

#endif //!_USE_MEGA_ESP
