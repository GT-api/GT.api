
void punch(ENetEvent event, state state) 
{
    if (not create_rt(event, 0, 120ms)) return; // this will only affect hackers (or macro spammers)
    short block1D = state.punch[1] * 100 + state.punch[0]; // 2D (x, y) to 1D ((destY * y + destX)) formula
    block& b = worlds[_peer[event.peer]->recent_worlds.back()].blocks[block1D];
    if (state.id == 18) // @note punching a block
    {
        if (b.bg == 0 and b.fg == 0) return;
        if (b.fg == 8 or b.fg == 6) // @todo sudden crash. this does not happen everytime??
        {
            gt_packet(*event.peer, false, "OnTalkBubble", _peer[event.peer]->netid, b.fg == 8 ? 
                "It's too strong to break." : "(stand over and punch to use)", 0, 1);
            return;
        }
        block_punched(event, state, block1D); // TODO- referance it?
        if (b.fg not_eq 0 and b.hits[0] >= items[b.fg].hits) b.fg = 0;
        else if (b.bg not_eq 0 and b.hits[1] >= items[b.bg].hits) b.bg = 0;
        else return;
        engine::simple random;
        if (random.uint32({0, 4}) == 1) 
            drop_visuals(event, {112, 1}, // @todo get real gt gem amount for each item.
            {static_cast<float>(state.punch[0]) + random.float32({0.08, 0.6}), static_cast<float>(state.punch[1]) + random.float32({0.08, 0.6})});
        // @todo add real gt "farmable" or just seed/block chances.
    }
    else // @note placing a block
    {
        if (state.punch[0] not_eq static_cast<int>(_peer[event.peer]->pos[0]) or state.punch[1] not_eq static_cast<int>(_peer[event.peer]->pos[1]))
            (items[state.id].type == 18) ? b.bg = state.id : b.fg = state.id; // @note this helps prevent foregrounds to act as backgrounds.
        else return;
        _peer[event.peer]->emplace({
            static_cast<short>(state.id),
            -1 // @note remove that item the peer just placed.
        });
    }
    auto w = std::make_unique<world>(worlds[_peer[event.peer]->recent_worlds.back()]);
    state_visuals(event, std::move(state)); // finished.
}