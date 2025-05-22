#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "equip.hpp"

void equip(ENetEvent event, state state)
{
    /* checks if item is clothing. */
    if (items[state.id].cloth_type != clothing::none) {

        /* checks if clothing is already equipped. if so unequip. else equip. */
        if (_peer[event.peer]->clothing[items[state.id].cloth_type] == state.id)
            _peer[event.peer]->clothing[items[state.id].cloth_type] = 0;
        else
            _peer[event.peer]->clothing[items[state.id].cloth_type] = state.id;

        clothing_visuals(event);
    }
    else {
        // @todo 100 world locks -> 1 diamond lock
    }
}
