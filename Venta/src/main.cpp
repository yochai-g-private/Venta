
#define APP_NAME        "Venta"
#define APP_VERSION     1

#if _USE_MEGA_ESP
#include "MegaEsp.h"

static bool OK = false;

//------------------------------------------------------------------------------------------
struct AppCfg : public MegaEsp::Cfg
{
    AppCfg() : MegaEsp::Cfg(APP_NAME, APP_VERSION)  {   }

private:

    Root& GetRoot()                                 { return root; }
    MegaEsp::Cfg::Cfg_2_Root root;
};
//------------------------------------------------------------------------------------------
class Callback : public MegaEsp::ICallback
{
    bool GetLogoText(const char*& row_1, const char*& row_2)    const
    {
        row_1 = " Venta & Lights";
        row_2 = " Control System";

        return true;
    }

    void AddDevices()
    {
    }

} callback;
//------------------------------------------------------------------------------------------
void setup() 
{
    static AppCfg   cfg;

    OK = MegaEsp::OnSetup(cfg, NULL, callback);

 //   MegaEsp::Beep(500, OK ? 1 : 10);
}
//------------------------------------------------------------------------------------------
void loop() 
{
    MegaEsp::OnLoop();

    if(!OK)
        return;
}
//------------------------------------------------------------------------------------------

const char*	gbl_build_date = __DATE__;
const char*	gbl_build_time = __TIME__;

#else
#endif // !_USE_MEGA_ESP
