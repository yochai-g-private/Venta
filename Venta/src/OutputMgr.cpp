#include "OutputMgr.h"
#include "TimeEx.h"

#define TESTING     1

#if TESTING
    #define TIMER_FACTOR        10UL
    #define WAKEUP_MINUTES      5
#else
    #define TIMER_FACTOR       (uint32_t) SECONDS_PER_MINUTE
    #define WAKEUP_MINUTES      120
#endif //TESTING

static uint8_t  st_storage_light_delay_minutes  = 5;

static uint8_t  st_venta_start_hour             = 8;
static uint8_t  st_venta_end_hour               = 20;
static uint8_t  st_venta_off_delay_minutes      = 5;
static uint8_t  st_wakeup_minutes               = WAKEUP_MINUTES;

static Relay    st_main_relay(MAIN_RELAY_PIN);
static Relay    st_light_relay(LIGHT_RELAY_PIN);
static Relay    st_venta_relay(VENTA_RELAY_PIN);
static Relay*   st_relays[] = { &st_main_relay, &st_venta_relay, &st_light_relay };

static Scheduler::Handler   st_light_scheduler_handler;
static Scheduler::Handler   st_venta_scheduler_handler;
static Scheduler::Handler   st_wakeup_handler;

static void update();
static void set_initial_light_state();
static bool set_light_relay();
static void set_initial_venta_state();
static bool set_venta_relay();

#define DEBUG_ME 0
#if DEBUG_ME
    #define MYTRACE()         _LOGGER << "YOCHAI: " << __LINE__ << NL
#else
    #define MYTRACE()
#endif

//--------------------------------------------------------------------------
void InitializeOutputs()
{
    st_main_relay.On();

    set_initial_light_state();
    set_light_relay();

    set_initial_venta_state();
    set_venta_relay();

    update();
}
//--------------------------------------------------------------------------
static void set_initial_light_state()
{
    gbl_state.light_state = false;
}
//--------------------------------------------------------------------------
static bool set_light_relay()
{
    if(st_light_relay.Set(gbl_state.light_state))
    {
        LOGGER << "Light set to " << ONOFF(gbl_state.light_state) << NL;
        return true;
    }

    return false;
}
//--------------------------------------------------------------------------
static bool set_light_relay(bool on)
{
    gbl_state.light_state = on;
    return set_light_relay();
}
//--------------------------------------------------------------------------
static void light_off(void*)
{
    set_light_relay(false);
}
//--------------------------------------------------------------------------
static void set_initial_venta_state()
{
    if(gbl_state.is_kodesh || Scheduler::IsScheduled(st_wakeup_handler))
    {
        gbl_state.venta_state = false;
        return;
    }

    DstTime  now  = DstTime::Now();
    uint32_t hour = now.GetHour();

    gbl_state.venta_state = hour >= st_venta_start_hour && hour < st_venta_end_hour;
}
//--------------------------------------------------------------------------
static bool set_venta_relay()
{
    if(st_venta_relay.Set(gbl_state.venta_state))
    {
        LOGGER << "Venta set to " << ONOFF(gbl_state.venta_state) << NL;
        return true;
    }

    return false;
}
//--------------------------------------------------------------------------
static bool set_venta_relay(bool on)
{
    gbl_state.venta_state = on;
    return set_venta_relay();
}
//--------------------------------------------------------------------------
static void venta_on(void*)
{
    if(gbl_state.is_kodesh)
        return;

    if(gbl_state.light_detected || gbl_state.room_motion_detected || gbl_state.storage_motion_detected)
        return;

    if(Scheduler::IsScheduled(st_wakeup_handler))
        return;

    set_initial_venta_state();

    set_venta_relay();
}
//--------------------------------------------------------------------------
static void update()
{
    if(gbl_state.is_kodesh)
    {
        set_light_relay(false);
        set_venta_relay(false);

        Scheduler::Cancel(st_light_scheduler_handler);
        Scheduler::Cancel(st_venta_scheduler_handler);
        Scheduler::Cancel(st_wakeup_handler);

        return;
    }

    static State state;
    if(!objequal(state, gbl_state))
    {
        if(gbl_state.light_detected && !state.light_detected)
            Scheduler::Cancel(st_wakeup_handler);

        state = gbl_state;
        ShowState();   
    }

    set_initial_light_state();

    if(gbl_state.storage_motion_detected)  //gbl_state.room_motion_detected
    {
        Scheduler::Cancel(st_light_scheduler_handler);
        gbl_state.light_state = true;
    }
    else
    if(st_light_relay.IsOn())
    {
        if(!Scheduler::IsScheduled(st_light_scheduler_handler))
        {
            MegaEsp::AddToScheduler(st_light_scheduler_handler, light_off, "Set Light OFF", TIMER_FACTOR * (uint32_t)st_storage_light_delay_minutes);
        }

        gbl_state.light_state = true;
    }

    set_light_relay();

    set_initial_venta_state();

    if(gbl_state.venta_state)
    {
        if(gbl_state.storage_motion_detected || gbl_state.room_motion_detected || gbl_state.light_detected)
        {
            Scheduler::Cancel(st_venta_scheduler_handler);
            gbl_state.venta_state = false;
        }
        else
        if(st_venta_relay.IsOff())
        {
            if(!Scheduler::IsScheduled(st_venta_scheduler_handler))
            {
                MegaEsp::AddToScheduler(st_venta_scheduler_handler, venta_on, "Set Venta ON", TIMER_FACTOR * (uint32_t)st_venta_off_delay_minutes);
            }

            gbl_state.venta_state = false;
        }
    }
    else
    {
        Scheduler::Cancel(st_venta_scheduler_handler);
    }

    set_venta_relay();
}
//--------------------------------------------------------------------------
static struct OutputMgr : IMgr          // copied from DoorMgr
{
    const char* GetName()   const { return "OutputMgr"; }

    void OnLoop_WriteOutputs()
    {
        update();
    }

    bool OnLoop_DoWithSerialData(const String& s)                       
    {
  // TODO        
      //_LOGGER << "DoorMgr::OnLoop_DoWithSerialData" << NL;
//        return  st_DoorOpen.DoWithSerialData(s) ||
//                st_Motion.  DoWithSerialData(s);
        return false;
    }
/*    
    bool OnSetup()
    {
        return true;
    }

    void OnLoop_TreateTimers()                                          
    { 
    }

    void OnLoop_ReadInputs()
    {
    }
*/
}   st_mgr;
//--------------------------------------------------------------------------
IMgr& gbl_OutputMgr = st_mgr;
//--------------------------------------------------------------------------
bool GetStorageLightState()
{
    return st_light_relay.IsOn();
}
//--------------------------------------------------------------------------
bool GetVentaState()
{
    return st_venta_relay.IsOn();
}
//--------------------------------------------------------------------------
static struct StorageLightDelayMinutes : public Cfg::NumericLeaf<uint8_t>
{
    const char* GetName()  const    { return "light_delay"; }
    uint8_t& GetValue()    const    { return st_storage_light_delay_minutes; }
}   st_StorageLightDelayMinutes;
//--------------------------------------------------------------------------
static struct Venta_StartHour : public Cfg::NumericLeaf<uint8_t>
{
    const char* GetName()  const    { return "venta_start"; }
    uint8_t& GetValue()    const    { return st_venta_start_hour; }
}   st_VentaStartHour;
//--------------------------------------------------------------------------
static struct Venta_EndHour : public Cfg::NumericLeaf<uint8_t>
{
    const char* GetName()  const    { return "venta_end"; }
    uint8_t& GetValue()    const    { return st_venta_end_hour; }
}   st_VentaEndHour;
//--------------------------------------------------------------------------
static struct VentaOff_DelayMinutes : public Cfg::NumericLeaf<uint8_t>
{
    const char* GetName()  const    { return "venta_off_delay"; }
    uint8_t& GetValue()    const    { return st_venta_off_delay_minutes; }
}   st_VentaDelayMinutes;
//--------------------------------------------------------------------------
static struct OutputCfg : Cfg::Node
{
    const char* GetName()  const    { return "output"; }

    OutputCfg()    {   }

    DECLARE_CFG_NODE_ITERATOR_FUNCS_4((Cfg::Item&)st_StorageLightDelayMinutes, (Cfg::Item&)st_VentaStartHour, (Cfg::Item&)st_VentaEndHour, (Cfg::Item&)st_VentaDelayMinutes);
}   st_OutputCfg;
//--------------------------------------------------------------------------
Cfg::Item&  GetOutputCfg()
{
    return st_OutputCfg;
}
//--------------------------------------------------------------------------
void ToggleRelay(int n)
{
    if(n < 0 || n > countof(st_relays))
        return;

    st_relays[n]->Toggle();
    delay(5000);
    st_relays[n]->Toggle();
}
//--------------------------------------------------------------------------
void iWantToSleep()
{
    set_venta_relay(false);
    Scheduler::Cancel(st_venta_scheduler_handler);
    MegaEsp::AddToScheduler(st_wakeup_handler, venta_on, "Wakeup", (uint32_t)SECONDS_PER_MINUTE * (uint32_t)st_wakeup_minutes);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
