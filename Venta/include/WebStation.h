#pragma once

#if !_USE_MEGA_ESP
class WebStation
{
public:

    static bool Begin(bool log = false);
    static bool UpdateCurrentTime();

};
#endif //!_USE_MEGA_ESP
