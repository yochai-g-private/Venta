#define APP_NAME        "Venta"
#define APP_VERSION     1

#include "main.h"
#include "InputMgr.h"
#include "OutputMgr.h"
#include "StdIR.h"

static bool OK = false;
static IMgr** st_mgrs;

State gbl_state = { 0 };

static void showHumidityAndTemperature();

//------------------------------------------------------------------------------------------
struct AppCfg : public MegaEsp::Cfg
{
    AppCfg() : MegaEsp::Cfg(APP_NAME, APP_VERSION)  {   }

private:

    struct  MegaEsp_Root : public  MegaEsp::Cfg::Cfg_2_Root    
    {
        const char* GetName()  const { return "MegaEsp"; }
    };

    struct AppRoot : public MegaEsp::Cfg::Root
    {
        MegaEsp_Root    mega_esp;
/*
        struct AppCfg : Cfg::Node
        {
            AppCfg()  : storage_light(GetStorageLightCfg()),
                        venta(GetVentaCfg()),
                        motion(GetMotionCfg())
            {
            }

            const char* GetName()  const { return "App"; }

            Cfg::Item&      storage_light;
            Cfg::Item&      venta;
            Cfg::Item&      motion;

            DECLARE_CFG_NODE_ITERATOR_FUNCS_3(storage_light, venta, motion);
        } app;
*/
        DECLARE_CFG_NODE_ITERATOR_FUNCS_2(mega_esp, GetOutputCfg());
    } root;

    Root& GetRoot()     { return root; }
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
        for(int idx = 0; st_mgrs[idx]; idx++)
            MegaEsp::Add(*st_mgrs[idx]);
    }

    bool OnSetup()
    {
        OnSunChangedEvent(MegaEsp::IsDay());
        OnKodeshChangedEvent(MegaEsp::IsKodesh());

        InitializeInputs();
        InitializeOutputs();

        //ShowState();

        return true;
    }

    void OnSecondChangedEvent()
    {
    }

    void OnKodeshChangedEvent(bool is_kodesh)                           
    { 
        LOGGER << "Setting Kodesh to " << is_kodesh << NL;

        gbl_state.is_kodesh = is_kodesh;

        if(is_kodesh)
        {
            gbl_state.room_motion_detected    =
            gbl_state.storage_motion_detected = 
            gbl_state.light_detected          = false;
        }
    }

    void OnSunChangedEvent(bool is_sunrise)                             
    { 
        LOGGER << "Setting Day to " << is_sunrise << NL;
        gbl_state.is_day = is_sunrise; 
    }

    bool OnLoop_DoWithSerialData(const String& s)                       
    {
#if 0
        // Simulate 
        if(s=="simulate")       gbl_state.simulation = true;        else
        if(s=="nosimulate")     gbl_state.simulation = false;       else
        if(s=="state")          ;                                   else
        if(s=="day")            gbl_state.is_day    = true;         else
        if(s=="night")          gbl_state.is_day    = false;        else
        if(s=="kodesh")         gbl_state.is_kodesh = true;         else
        if(s=="chol")           gbl_state.is_kodesh = false;        else
        return false; 

        ShowSimulationState(s);
#endif
        return true;
    }

    bool DisplayOnTheScreen()   const
    {
        static int cnt = 0;

        switch(cnt)
        {
            case 0 :
            {
                char line[33];
                strcpy(line, "Temperat: "); dtostrf(GetEnvTemperature(), 2, 2, &line[strlen(line)]);
                MegaEsp::ShowOnScreen(line, 0, true);
                strcpy(line, "Humidity: "); dtostrf(GetEnvHumidity(), 2, 2, &line[strlen(line)]);
                MegaEsp::ShowOnScreen(line, 1, false);

                break;
            }

            default :
                cnt = 0;
                return false;
        }

        cnt++;
        return true;
    }

    bool TreateIrKey(StdIR::Key key)
    {
        static const char title[] = "Application Menu";
        bool ok = true;

        switch(key)
        {
            case INVALID_IR_KEY : 
                break;

            case StdIR::OK : 
                iWantToSleep();
                break;

            case StdIR::N0 : 
                ToggleRelay(0);
                break;

            case StdIR::N1 : 
                ToggleRelay(1);
                break;

            case StdIR::N2 : 
                ToggleRelay(2);
                break;

            case StdIR::DIEZ :
                showHumidityAndTemperature();
                break;

            default : 
                MegaEsp::ShowOnScreen2(title, "Key not in use!");
                delay(2000);
                ok = false;
        }

        MegaEsp::ShowOnScreen2(title, "OK, 0-2, #");

        return ok;
    }
} callback;
//------------------------------------------------------------------------------------------
void setup() 
{
    static IMgr* managers[] = {&gbl_InputMgr, &gbl_OutputMgr, NULL };
    st_mgrs = managers;


    static AppCfg   cfg;

    OK = MegaEsp::OnSetup(cfg, NULL, callback);
}
//------------------------------------------------------------------------------------------
void loop() 
{
    MegaEsp::OnLoop();

    if(!OK)
        return;
}
//------------------------------------------------------------------------------------------
static void show_state_field(const char* name, const bool& value, bool onoff)
{
    _LOGGER << "    " << name << ": ";
    
    if(onoff)   _LOGGER     << ONOFF(value).Get()   << NL;
    else        _LOGGER     << value                << NL;
}
//------------------------------------------------------------------------------------------
template <class T>
static void show_state_field(const char* name, const T& value)
{
    _LOGGER << "    " << name << ": " << value << NL;
}
//------------------------------------------------------------------------------------------
void ShowState()        
{
    _LOGGER << "State: " << NL;

    #define SHOW_BOOL_FLD(fld)     show_state_field( #fld, gbl_state.fld, false )
    #define SHOW_ONOFF_FLD(fld)    show_state_field( #fld, gbl_state.fld, true )
    #define SHOW_FLD(fld)          show_state_field( #fld, gbl_state.fld )

//    SHOW_ONOFF_FLD(simulation);
    SHOW_BOOL_FLD(is_day);
    SHOW_BOOL_FLD(is_kodesh);
    SHOW_BOOL_FLD(room_motion_detected);
    SHOW_BOOL_FLD(storage_motion_detected);
    SHOW_BOOL_FLD(light_detected);

    show_state_field("Light", GetStorageLightState(), true);
    show_state_field("Venta", GetVentaState(),        true);

    #undef  SHOW_BOOL_FLD
    #undef  SHOW_ONOFF_FLD
}
//------------------------------------------------------------------------------------------
static void showHumidityAndTemperature()
{
    static bool H = false;
    H = !H;

    char s[16];
    if(H)   sprintf(s, "h%3d", (int) GetEnvHumidity());
    else    sprintf(s, "t%3d", (int) GetEnvTemperature());

    MegaEsp::GetSSD().PrintText(s);

    delay(5000);
}

const char*	gbl_build_date = __DATE__;
const char*	gbl_build_time = __TIME__;
