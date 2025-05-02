#include "action/quit.hpp"

void type_disconnect(ENetEvent event)
{
    quit(event, "");
}