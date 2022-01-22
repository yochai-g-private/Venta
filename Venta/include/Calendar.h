#pragma once

#if !_USE_MEGA_ESP

#include <TimeEx.h>
#include <Scheduler.h>

class Calendar
{
public:

    static void TestLeds();
    static void Initialize();
    static void Update(const Times& times);
    static bool AddToScheduler(const FixTime& now, Scheduler::Handler& handler, void (*action)(void*), const char* description, const FixTime& time);
    static void ScheduleRestart(Scheduler::Handler& handler, const char* description, const FixTime& restart_time);
    static void OnLoop();
};

#endif //!_USE_MEGA_ESP
