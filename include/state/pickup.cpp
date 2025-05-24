#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "pickup.hpp"

void pickup(ENetEvent event, state state) 
{
    printf("%d", state.id);
    std::vector<ifloat>& ifloats{worlds[_peer[event.peer]->recent_worlds.back()].ifloats};
    int x = std::lround(_peer[event.peer]->pos[0]);
    int y = std::lround(_peer[event.peer]->pos[1]);
    auto it = std::find_if(ifloats.begin(), ifloats.end(), [&](const ifloat& i) 
    {
        int ix = std::lround(i.pos[0]);
        int iy = std::lround(i.pos[1]);
        return (std::abs(ix - x) <= 1) && (std::abs(iy - y) <= 1);
    });

    if (it != ifloats.end()) 
    {
        short excess = _peer[event.peer]->emplace(slot{it->id, static_cast<short>(it->count)});
        short remember_count = it->count;
        it->count -= (it->count - excess);
        if (it->count == 0) 
        {
            drop_visuals(event, {it->id, it->count}, it->pos, state.id/*@todo*/);
            ifloats.erase(it);
            if (it->id != 112)
            {
                gt_packet(*event.peer, false, 0, {
                    "OnConsoleMessage",
                    std::format("Collected `w{} {}``. Rarity: `w{}``", remember_count, items[it->id].raw_name, items[it->id].rarity).c_str()
                });
                inventory_visuals(event);
            }
            else 
            {
                // @todo add gem system
            }
        }
    }
}