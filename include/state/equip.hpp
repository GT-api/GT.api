
void equip(ENetEvent event, state state)
{
    if (items[state.id].cloth_type == clothing::none) return;

    _peer[event.peer]->clothing[items[state.id].cloth_type] = state.id;

    clothing_visuals(event);
}
