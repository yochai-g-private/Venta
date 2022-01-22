#if !_DM_FROM_INTERNET_NOT_SUPPORTED
#include "Shaon.h"
#include "WebStation.h"
#include "Calendar.h"

#include <EspAT.h>
#include <RTC.h>
#include <TimeZoneData.h>
#include <SmartHomeWiFiApp.h>
#include <SmartHomeWiFiAppDefaults.h>
#include <WiFiUtils.h>
#include <Scheduler.h>
#include <SmartHomeWiFi.h>

static EspAT_Station* pStation;
#define STATION (*pStation)

static TimeZoneData st_tzd;

//==========================================================================
bool WebStation::Begin(bool log)
{
    EspAT::SerialIO sio = EspAT::SIO_3;
    EspAT::SetLogging(log);

    if(!EspAT::Reset(sio))
    {
        LOGGER << "Failed to reset ESP" << NL;
    }
    else
    {
        delay(5000);
    }

    const SmartHomeWiFi& sh = SmartHomeWiFi::GetDefault();

    const WiFi* APs[] = { &sh.main_WiFi, &sh.alternate_WiFi };

    for(uint8_t idx = 0; idx < countof(APs); idx++)
    {
        if(!*APs[idx]->SSID)
            continue;

        pStation = new EspAT_Station(sio, 
                                     APs[idx]->SSID, APs[idx]->pass, 
                                     0, true, false);

        EspAT_Station::State state = pStation->GetState();

        bool ok = state == EspAT_Station::CONNECTED;

        if(!ok)
        {
            delete pStation;
            pStation = NULL;

            if(state == EspAT_Station::TEST_FAILED)
            {
                LOGGER << "ESP-01 test failed" << NL;
                return false;
            }
            
            continue;
        }

        String IP;
        STATION->GetStationIP(IP);

        LOGGER << "Connected to the AP " << APs[idx]->SSID << ", IP=" << IP << NL;

        if (!UpdateCurrentTime())
        {
            LOGGER << "Failed to access TZ server" << NL;

            delete pStation;
            pStation = NULL;
            continue;
        }

        delete pStation;
        pStation = NULL;

        return true;
    }

    return false;
}
//==========================================================================
void ScheduleDstUpdate()
{?
    static Scheduler::Handler   st_RestartHandler;
    Calendar::ScheduleRestart(st_RestartHandler, (gbl_DST ? TEXT_WINTER : TEXT_SUMMER), gbl_DST_switch_at);
}
//==========================================================================
bool WebStation::UpdateCurrentTime()
{
    String answer;
/*
AT+CWMODE=1
AT+CWJAP=HOME_TEST_SSID,HOME_WIFI_COMMON_PASS
AT+CIPSTA?
AT+CIPSTART="TCP","api.timezonedb.com",80
AT+CIPSEND=153
GET http://api.timezonedb.com/v2/get-time-zone?key=FY1HFJBKCESS&format=xml&fields=gmtOffset,dst,dstStart,dstEnd,timestamp&by=zone&zone=Asia/Jerusalem

*/
    //STATION->DisableMultipleConnections();

    if (!STATION->SendHttpGET(TimeZoneData::GetServerAddress(), TimeZoneData::GetServerPort(), TimeZoneData::GetRelativeURL(), answer, 0, false))
    {
        LOGGER << "Failed to access TZ server" << NL;
        return false;
    }
    
    TimeZoneData tzd(answer);

    if (!tzd.Ok)
    {
        LOGGER << "Bad answer received from the TZ server" << NL;
        return false;
    }

    gbl_DST = tzd.dst;

    tzd.currentTime += 1;
    
    if(!RTC::Set(tzd.currentTime)) 
        FixTime::Set(tzd.currentTime);

    LOGGER << "Current time: " << DstTime(tzd.currentTime).ToText() << NL;
    LOGGER << "DST         : " << (tzd.dst ? "On" : "Off") << NL;
    LOGGER << "DST start   : " << DstTime(tzd.dstStart).ToText() << NL;
    LOGGER << "DST end     : " << DstTime(tzd.dstEnd).ToText() << NL;

    st_tzd = tzd;

    FixTime restart_when_DST_changes = tzd.dstStart;

    if (restart_when_DST_changes < tzd.currentTime)
        restart_when_DST_changes = tzd.dstEnd;
    
    uint32_t minutes = restart_when_DST_changes.GetMinutes() % MINUTES_PER_HOUR;

    if(minutes)
        restart_when_DST_changes += (MINUTES_PER_HOUR - minutes) * SECONDS_PER_MINUTE;
    
    restart_when_DST_changes += 2 * SECONDS_PER_HOUR;
    
    gbl_DST_switch_at = restart_when_DST_changes;

    ScheduleDstUpdate();
    StoreCfg();
    
    return true;
}
//==========================================================================
#endif //!_DM_FROM_INTERNET_NOT_SUPPORTED
