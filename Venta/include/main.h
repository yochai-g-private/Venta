#pragma once

#include "Logger.h"
#include "IInput.h"
#include "IOutput.h"
#include "MegaEsp.h"

#define TESTING     0

enum Pins
{
    MOTION_SENSORS_SWITCH_MOSFET_PIN            = D3,

    ROOM_MOTION_SENSOR_PIN                      = D23,
    STORAGE_MOTION_SENSOR_PIN                   = A3,
    MAIN_RELAY_PIN                              = A9,
    LIGHT_RELAY_PIN                             = A11,
    VENTA_RELAY_PIN                             = A13,

    LIGHT_SENSOR_PIN                            = D22,
};

typedef RevertedDigitalOutputPin        Relay;

typedef MegaEsp::IDevice IMgr;

extern IMgr& gbl_InputMgr,
           & gbl_OutputMgr;

struct State
{
    bool    simulation;
    bool    is_day;
    bool    is_kodesh;
    bool    room_motion_detected;
    bool    storage_motion_detected;
    bool    light_detected;

    bool    venta_state;
    bool    light_state;
};

extern State gbl_state;

void ShowState();
float GetEnvTemperature();
float GetEnvHumidity();