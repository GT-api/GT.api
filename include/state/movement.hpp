
void movement(ENetEvent event, state state) 
{
    if (getpeer->post_enter.try_lock()) // memory optimize- push only during an actual world enter
    {
        gt_packet(*event.peer, 0, true, "OnSetPos", floats{getpeer->pos[0], getpeer->pos[1]});
        gt_packet(*event.peer, 0, true, "OnChangeSkin", -1429995521);
    }
    getpeer->pos = state.pos;
    getpeer->facing_left = state.peer_state bitand 0x10;
    state_visuals(event, std::move(state)); // finished.
}