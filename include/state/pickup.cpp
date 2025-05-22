#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "pickup.hpp"

void pickup(ENetEvent event, state state) 
{
    std::vector<ifloat>& ifloats{worlds[_peer[event.peer]->recent_worlds.back()].ifloats};
    auto it = std::find_if(ifloats.begin(), ifloats.end(), [&](const ifloat& i) 
    {
        return std::abs(i.pos[0] - _peer[event.peer]->pos[0]) <= 0.6f &&
               std::abs(i.pos[1] - _peer[event.peer]->pos[1]) <= 0.6f;
    });

    if (it != ifloats.end()) 
    {
        gt_packet(*event.peer, false, 0, {
            "OnConsoleMessage",
            std::format("Collected `w{} {}``. Rarity: `w{}``", it->count, items[it->id].raw_name, items[it->id].rarity).c_str()
        });
        short excess = _peer[event.peer]->emplace(slot{it->id, static_cast<short>(it->count)});
        it->count -= (it->count - excess);
        if (it->count == 0) 
        {
            ifloats.erase(it);
        }
    }
}