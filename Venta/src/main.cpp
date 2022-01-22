
#define APP_NAME        "Home Clock"
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
        row_1 = " שעון שבת שלום!";
        row_2 = "*** GLAUBERS ***";

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
#if !TEST_HEB
    MegaEsp::OnLoop();

    if(!OK)
        return;
#endif
}
//------------------------------------------------------------------------------------------

const char*	gbl_build_date = __DATE__;
const char*	gbl_build_time = __TIME__;

#else
#include "Shaon.h"
#include "WebStation.h"
#include "Screen.h"
#include "Calendar.h"
#include <Location.h>
#include <RTC.h>
#include <IInput.h>
#include <IOutput.h>
#include <Observer.h>
#include <TimeDisplay.h>
#include <Toggler.h>
#include <Scheduler.h>
#include <PushButton.h>
#include <Cfg.h>

#define LCD_ADDRESS				0x27
#define LCD_LINE_LEN			16
#define LCD_N_ROWS				2

//*******************************************************
// INPUT DEVICES
//*******************************************************
static DigitalPullupInputPin    ___1Hz_ticker(_1HZ_TICKER_PIN);
static DigitalObserver          st_1Hz_ticker_observer(___1Hz_ticker);

static PullupPushButton         ___button(BUTTON_PIN);
static DigitalObserver          st_button_observer(___button);

//*******************************************************
// OUTPUT DEVICES
//*******************************************************
static TimeDisplay              st_time_display(SEVEN_SEGMENT_DISPLAY_CLK_PIN, SEVEN_SEGMENT_DISPLAY_DIO_PIN);
static DigitalOutputPin         ___buzzer(BUZZER_PIN);
static Toggler                  st_buzzer_toggler;
static DigitalOutputPin         st_builtin_led(LED_BUILTIN);

static bool OK = false;

const char*	gbl_build_date = __DATE__;
const char*	gbl_build_time = __TIME__;

//------------------------------------------------------------------------------------------
namespace NYG   
{
     Cfg::Item& GetLocationCfg(); 
     Cfg::Item& GetTimeExCfg(); 
	 Cfg::Item& GetKodeshCfg();
     Cfg::Item& GetWifiCfg();
}
//------------------------------------------------------------------------------------------
struct AppCfg : public Cfg
{
    AppCfg() : Cfg(APP_NAME, APP_VERSION)     {   }

private:

    struct Root : Cfg::NodeBase
    {
        Root() : location(GetLocationCfg()), time_ex(GetTimeExCfg()), kodesh(GetKodeshCfg()), wifi(GetWifiCfg())
        {
        }

        Cfg::Item& location;
        Cfg::Item& time_ex;
        Cfg::Item& kodesh;
        Cfg::Item& wifi;

        DECLARE_CFG_NODE_ITERATOR_FUNCS_4(location, time_ex, kodesh, wifi);
    };

    NodeBase& GetRoot()     { return root; }

    Root root;
};

static AppCfg*  p_Cfg = NULL;
#define cfg (*p_Cfg)

static void doWithSerialData();

static Toggler st_builtin_led_toggler;
//-----------------------------------------------------------------------
void StoreCfg()
{
    cfg.Store();
}
//-----------------------------------------------------------------------
void setup() 
{
    Logger::Initialize();

    static AppCfg st_cfg;
    p_Cfg = &st_cfg;

    cfg.Load();

    Screen::Initialize();
    
    String app_name = "*  " APP_NAME               "  *";
    String app_ver  = String("*  version #") + String(APP_VERSION) +  "  *";

    Screen::Show(app_name.c_str(), app_ver.c_str());

    delay(1000);

    Calendar::TestLeds();

    const char frame[] = "****************";
    String app_version(APP_VERSION);

    _LOGGER << NL << frame  << NL;
    _LOGGER << app_name     << NL;
    _LOGGER << app_ver      << NL;
    _LOGGER << frame        << NL << NL;

    _LOGGER << "Built: " << gbl_build_date << " " << gbl_build_time << NL;
    
    _LOGGER << "Configuration:" << NL;
    cfg.Show();

    _LOGGER << "Initializing RTC..." << NL;

    ShowScreenMessage("Initializing RTC...", "אתחל שעון");
    delay(1000);

    if(RTC::Begin())
    {
        st_time_display.Update();
        LOGGER << "RTC initialization done." << NL;

        LOGGER << "Initializing 1Hz oscillator..." << NL;
        RTC::EnableOscillator();
        st_1Hz_ticker_observer.TestChanged();

        ScheduleDstUpdate();
    }
    else
    {
        _LOGGER << "Failed to initialize RTC." << NL;
        Screen::Show(" * RTC ERROR! * ");
        delay(5000);
    }

    LOGGER << "Initializing WEB Station..." << NL;
    ShowScreenMessage("Get Interet time", "אתחל רשת");

    if(WebStation::Begin())
    {
        st_time_display.Update();
        LOGGER << "WEB Station initialization done." << NL;
    }
    else
    {
        LOGGER << "Failed to initialize time from internet." << NL;
        Screen::Show("No Internet time");
        delay(5000);

        FixTime now = FixTime::Now();

        //if(now.IsValid())
        {
            FixTime restart_time = now + (15 * SECONDS_PER_MINUTE); //now.GetMidNight() + SECONDS_PER_DAY + (3 * SECONDS_PER_HOUR);

            static Scheduler::Handler   st_RestartHandler;
            Calendar::ScheduleRestart(st_RestartHandler, TEXT_RESTART, restart_time);
        }

        st_builtin_led_toggler.StartOnOff(st_builtin_led,125);
    }

    if(!FixTime::Now().IsValid())
    {
        Screen::Show("     ERROR    ",
                     " INVALID  TIME ");

        return;
    }

    OK = true;

    Calendar::Initialize();

    st_button_observer.TestChanged();

    ShowScreenMessage("Ready!", "מוכן!");

    delay(1000);

    SignalBuzzer(1000, 1);

    LOGGER << "Ready!" << NL;
}
//-----------------------------------------------------------------------
void loop() 
{
    st_buzzer_toggler.Toggle();
    st_builtin_led_toggler.Toggle();

    if(Serial.available())
        doWithSerialData();

    if (!OK)
        return;

    Scheduler::Proceed();

    Screen::OnLoop();
    Calendar::OnLoop();

    bool on;

    if(st_button_observer.TestChanged(on) && !on)
        st_time_display.SetNextMode();

    if(st_1Hz_ticker_observer.TestChanged(on))
    {
        if(!st_builtin_led_toggler.IsStarted())
            st_builtin_led.Set(on);

        DstTime now = DstTime::Now();
        Times times(now);
        
        st_time_display.Update(times);

        if(on)
        {
            Calendar::Update(times);
        }

        static hour_t prev_hour = 0xFF;

        if(prev_hour != times.h)
        {
            prev_hour = times.h;

            if(!st_buzzer_toggler.IsStarted())
            {
                uint16_t n_times = 1 + (times.h == 0);
                SignalBuzzer(250, n_times);
            }
        }
    }
}
//-----------------------------------------------------------------------
void SignalBuzzer(uint16_t on_off_millis, uint8_t n_times)
{
    st_buzzer_toggler.StartOnOff(___buzzer, on_off_millis, n_times);
}
//-----------------------------------------------------------------------
static void doWithSerialData()
{
    String s = Serial.readString();
    
    s.trim();

    bool consumed = cfg.DoCommand(s);

    if(consumed)
        return;

    if(s == "now" || s == "time")
    {
        _LOGGER << s << "=" << DstTime::Now().ToText() << NL;
    } else
    if(s == "scheduled")
    {
        Scheduler::ScheduledItem* scheduled = Scheduler::GetFirst();

        if(!scheduled)
        {
            _LOGGER << "No scheduled events" << NL;
            return;
        }

        _LOGGER << "Scheduled events:" << NL;

        int cnt = 0;

        while(scheduled)
        {
            DstTime     t = scheduled->GetTime();
            const char* d = scheduled->GetDescription();
            TimeText tt = t.ToText();

            cnt++;

            _LOGGER << "  " << cnt << ". " << tt << " : " << d << NL;

            scheduled = scheduled->GetNext();
        }
    } else
    {
        LOGGER << "Unknown command '" << s << "'" << NL;
    }
}
#endif // !_USE_MEGA_ESP
