
#include "InputMgr.h"
#include "AHT.h"
#include "Average.h"
#include "StableInput.h"

typedef StableDigitalInput<200, 200, millis>    StableLightSensor;

static DigitalOutputPin     st_motion_sensors_switch(MOTION_SENSORS_SWITCH_MOSFET_PIN);
static MotionSensor         st_room_motion_sensor(ROOM_MOTION_SENSOR_PIN);
static MotionSensor         st_storage_motion_sensor(STORAGE_MOTION_SENSOR_PIN);
#if 1
static DigitalLightSensor   st_light_sensor(LIGHT_SENSOR_PIN);
#else
static DigitalLightSensor   _light_sensor(LIGHT_SENSOR_PIN);
static StableLightSensor    st_light_sensor(_light_sensor);
#endif
static AHT                  st_aht;

static void update();
//--------------------------------------------------------------------------
void InitializeInputs()
{
    if(!st_aht.Initialize())
        LOGGER << "AHT failed to initialize" << NL;

    update();
}
//--------------------------------------------------------------------------
static void update_humidity_and_temperature()
{
    if(!st_aht.OK())
        return;

    static unsigned long last_sample_millis = 0;
    unsigned long sample_millis = millis();

    if((sample_millis - last_sample_millis) < 100)
        return;

    last_sample_millis = sample_millis;

    st_aht.Update();
}
//--------------------------------------------------------------------------
float GetEnvTemperature()   { return st_aht.GetTemperature(); }
float GetEnvHumidity()      { return st_aht.GetHumidity(); }
//--------------------------------------------------------------------------
#define set_state(fld, val)     \
    on = val; \
    if(on != gbl_state.fld) { gbl_state.fld = on; LOGGER << #fld " set to " << on << NL; } else
//--------------------------------------------------------------------------
static void update()
{
    update_humidity_and_temperature();

    if(gbl_state.is_kodesh)
    {
        st_motion_sensors_switch.Off();
        gbl_state.room_motion_detected      =
        gbl_state.storage_motion_detected   =
        gbl_state.light_detected            = 0;

        return;
    }

    static unsigned long sensor_evaluation_delayed;

    if(st_motion_sensors_switch.Set(true))
    {
        sensor_evaluation_delayed = millis();
        if(!sensor_evaluation_delayed)
            sensor_evaluation_delayed = 1;

        return;
    }

    if(sensor_evaluation_delayed)
        if(millis() - sensor_evaluation_delayed < 5000)
            return;
        else
            sensor_evaluation_delayed = 0;
    
    bool on;
    set_state(room_motion_detected      , st_room_motion_sensor.Get());
    //set_state(storage_motion_detected   , st_storage_motion_sensor.Get());
    set_state(light_detected            , st_light_sensor.Get());
}
//--------------------------------------------------------------------------
static struct InputMgr : IMgr          // copied from DoorMgr
{
    const char* GetName()   const { return "InputMgr"; }

    void OnLoop_ReadInputs()
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

    void OnLoop_WriteOutputs()
    {
    }
*/
}   st_mgr;
//--------------------------------------------------------------------------
IMgr& gbl_InputMgr = st_mgr;
//------------------------------------------------------------------------------------------
/*
static uint8_t              st_delay_minutes = 5;
//------------------------------------------------------------------------------------------
static struct MotionDelayMinutes : public Cfg::NumericLeaf<uint8_t>
{
    const char* GetName()  const    { return "motion_delay_minutes"; }
    uint8_t& GetValue()    const    { return st_delay_minutes; }
}   st_DelayMinutes;
//--------------------------------------------------------------------------
Cfg::Item&  GetMotionCfg()
{
    return st_DelayMinutes;
}
*/
//--------------------------------------------------------------------------

