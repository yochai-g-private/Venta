#if !_USE_MEGA_ESP
#include "Calendar.h"
#include "Shaon.h"

#include <Sun.h>
#include <Kodesh.h>
#include <Screen.h>
#include <RGB_Led.h>
#include <TimeEx.h>
#include <MicroController.h>

static RGB_Led  led(LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);

#define LED_FREQUENCY_SECONDS   10
#define LED_FREQUENCY           (LED_FREQUENCY_SECONDS * 1000)

static void update();

static char is_day,
            is_kodesh;

//------------------------------------------------------------
void Calendar::TestLeds()
{
    led.GetRed().On();      delay(500); led.SetOff();
    led.GetGreen().On();    delay(500); led.SetOff();
    led.GetBlue().On();     delay(500); led.SetOff();
}
//------------------------------------------------------------
void Calendar::Initialize()
{
    update();
    Calendar::OnLoop();
}
//----------------------------------------------------------
void Calendar::OnLoop()
{
    static uint32_t seconds;
    
    uint32_t now = FixTime::Now().GetSeconds();

    if(seconds == now)
        return;

    seconds = now;

    if(now % LED_FREQUENCY_SECONDS)
        return;

    if(is_kodesh)
    {
        led.GetRed().On(); delay(LED_SHORT_BLINK); led.SetOff();
    }

    IDigitalOutput& out = (is_day) ? led.GetGreen() : led.GetBlue();

    out.On(); delay(LED_SHORT_BLINK); out.Off();
}
//----------------------------------------------------------
void Calendar::Update(const Times& times)
{
    static minute_t prev_minute = 0xFF;

    if(prev_minute != times.m)
    {
        prev_minute = times.m;
        update();
    }

    static second_t prev_refresh = 0xFF;
    second_t refresh = times.s / 5;

    if(prev_refresh == refresh)
        return;

    prev_refresh = refresh;  

    static int event_idx = 0;

    Scheduler::ScheduledItem* scheduled = Scheduler::GetFirst();

    for(int idx = 0; scheduled && idx < event_idx; idx++)
        scheduled = scheduled->GetNext();

    if(scheduled)
    {
        event_idx++;
        DstTime     t = scheduled->GetTime();
        const char* d = scheduled->GetDescription();
        TimeText tt = t.ToText();

#if _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
        Screen::Show(tt);
        Screen::ShowHeb(d, 1, false);
#else
        Screen::Show(tt, d);
#endif //        _USE_HEBREW_TEXT_WITH_LiquidCrystalDisplay
    }
    else
    {
        event_idx = 0;
        Screen::ShowLogo();
    }
}
//----------------------------------------------------------
static void onSunRise(void* ctx)
{
    SignalBuzzer(250, 2);
    is_day = true;
    update();
}
//----------------------------------------------------------
static void onSunSet(void* ctx)
{
    SignalBuzzer(250, 2);
    is_day = false;
    update();
}
//----------------------------------------------------------
static void onHadlaka(void* ctx)
{
    SignalBuzzer(1000, 3);
    is_kodesh = true;
    update();
}
//----------------------------------------------------------
static void onHavdala(void* ctx)
{
    SignalBuzzer(1000, 3);
    is_kodesh = false;
    update();
}
//----------------------------------------------------------
bool Calendar::AddToScheduler(const FixTime& now, Scheduler::Handler& handler, void (*action)(void*), const char* description, const FixTime& time)
{
    int32_t delta = time - now;

    if(delta <= 0)
        return false;

    int32_t max_delta = SECONDS_PER_DAY;
    max_delta *= 31 * 9;

    if(delta > max_delta)
        return false;

    Scheduler::Add(&handler, action, NULL, description, time);

    return true;
}
//----------------------------------------------------------
static void add_sun_events(const FixTime& now, const FixTime& ref, int& cnt)
{
    FixTime riseTime, setTime;

    Sun::GetLocalRiseSetTimes(ref, riseTime, setTime);
    
    static Scheduler::Handler   st_SunRise, st_SunSet;

    if(Calendar::AddToScheduler(now, st_SunRise, onSunRise, TEXT_ZERICHA, riseTime))
    {
        if(!cnt)
            is_day = false;

        cnt++;
    }

    if((cnt < 2) &&
       Calendar::AddToScheduler(now, st_SunSet,  onSunSet, TEXT_SHEKYIA,  setTime))
    {
        if(!cnt)
             is_day = true;
            
        cnt++;
    }
}
//----------------------------------------------------------
static void update()
{
    FixTime now = FixTime::Now();
    
    now += 1;

    int cnt = 0;

    add_sun_events(now, now, cnt);

    if(cnt < 2) add_sun_events(now, now + SECONDS_PER_DAY, cnt);

    FixTime hadlaka = Kodesh::GetNextHadlaka(now);
    FixTime havdala = Kodesh::GetNextMotzeyKodesh(now);

    static Scheduler::Handler   st_hadlaka, st_havdala;

    is_kodesh = false;
    if(hadlaka < havdala)
    Calendar::AddToScheduler(now, st_hadlaka, onHadlaka, TEXT_HADLAKA, hadlaka);
    else
    is_kodesh = true;
    
    Calendar::AddToScheduler(now, st_havdala, onHavdala, TEXT_HAVDALA, havdala);
}
//----------------------------------------------------------
static void restart(void* ctx)
{
    MicroController::Restart();
}
//----------------------------------------------------------
void Calendar::ScheduleRestart(Scheduler::Handler& handler, const char* description, const FixTime& restart_time)
{
    Calendar::AddToScheduler(FixTime::Now(), handler, restart, description, restart_time);
}
//----------------------------------------------------------
#endif //!_USE_MEGA_ESP
