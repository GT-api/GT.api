
void punch(ENetEvent event, state state) 
{
    if (not create_rt(event, 0, 120ms)) return; // this will only affect hackers (or macro spammers)
    short block1D = state.punch[1] * 100 + state.punch[0]; // 2D (x, y) to 1D ((destY * y + destX)) formula
    block& b = worlds[getpeer->recent_worlds.back()].blocks[block1D];
    if (state.id == 18) // punching blocks
    {
        // ... TODO add a timer that resets hits every 6-8 seconds (threaded stopwatch)
        if (b.bg == 0 and b.fg == 0) return;
        if (b.fg == 8 or b.fg == 6) 
        {
            gt_packet(*event.peer, 0, false, "OnTalkBubble", getpeer->netid, b.fg == 8 ? 
                "It's too strong to break." : "(stand over and punch to use)");
            return;
        }
        block_punched(event, state, block1D); // TODO- referance it?
        if (b.fg not_eq 0 and b.hits[0] >= items[b.fg].hits) b.fg = 0;
        else if (b.bg not_eq 0 and b.hits[1] >= items[b.bg].hits) b.bg = 0;
        else return;
    }
    else 
    {
        if (state.punch[0] not_eq static_cast<int>(getpeer->pos[0]) or state.punch[1] not_eq static_cast<int>(getpeer->pos[1]))
            (items[state.id].type == 18) ? b.bg = state.id : b.fg = state.id;
        else return; 
    }
    auto w = std::make_unique<world>(worlds[getpeer->recent_worlds.back()]);
    overwrite_tile(w, block1D, b);
    state_visuals(event, std::move(state)); // finished.
}