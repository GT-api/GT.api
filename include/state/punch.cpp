#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "punch.hpp"

void punch(ENetEvent event, state state) 
{
    if (not create_rt(event, 0, 120)) return; // this will only affect hackers (or macro spammers)
    short block1D = state.punch[1] * 100 + state.punch[0]; // 2D (x, y) to 1D ((destY * y + destX)) formula
    block& b = worlds[_peer[event.peer]->recent_worlds.back()].blocks[block1D];
    if (state.id == 18) // @note punching a block
    {
        if (b.bg == 0 and b.fg == 0) return;
        if (b.fg == 8 or b.fg == 6)
        {
            gt_packet(*event.peer, false, {
                "OnTalkBubble", 
                _peer[event.peer]->netid, 
                (b.fg == 8) ? "It's too strong to break." : "(stand over and punch to use)"
            });
            return;
        }
        block_punched(event, state, block1D); // TODO- referance it?
        if (b.fg not_eq 0 and b.hits[0] >= items[b.fg].hits) b.fg = 0;
        else if (b.bg not_eq 0 and b.hits[1] >= items[b.bg].hits) b.bg = 0;
        else return;
        if (rand() % 4 == 0)
        {
            drop_visuals(event, 
                {112, 1/* @todo get real gt gem amount for each item. */},
                {
                    static_cast<float>(state.punch[0]) + (0.08f + (static_cast<float>(rand()) / RAND_MAX) * (0.6f - 0.08f)), 
                    static_cast<float>(state.punch[1]) + (0.08f + (static_cast<float>(rand()) / RAND_MAX) * (0.6f - 0.08f))
                }
            );
        }
    }
    else // @note placing a block
    {
        // @todo check if peer is on the block placement.
        (items[state.id].type == 18) ? b.bg = state.id : b.fg = state.id; // @note this helps prevent foregrounds to act as backgrounds.
        _peer[event.peer]->emplace(slot{
            static_cast<short>(state.id),
            -1 // @note remove that item the peer just placed.
        });
    }
    auto w = std::make_unique<world>(worlds[_peer[event.peer]->recent_worlds.back()]);
    state_visuals(event, std::move(state)); // finished.
}