#include "action/quit.hpp"

inline void type_disconnect(ENetEvent event)
{
    quit(event, "");
}