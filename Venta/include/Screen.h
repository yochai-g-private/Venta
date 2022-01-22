#pragma once

#if !_USE_MEGA_ESP

#include <NYG.h>

class Screen
{
public:

    static void Initialize();

    static void Show(const char* s1, const char* s2);
    static void Show(const char* s);

#if _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
    static void ShowHeb(const char* s, uint8_t row, bool clear);
#endif //_USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay

    static void ShowLogo();

    static void OnLoop();

private:

};

#endif //!_USE_MEGA_ESP
