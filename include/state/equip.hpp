
void equip(ENetEvent event, state state)
{
    /* checks if item is clothing. */
    if (items[state.id].cloth_type == clothing::none) {

        /* checks if clothing is already equipped. if so unequip. else equip. */
        if (_peer[event.peer]->clothing[items[state.id].cloth_type] == state.id)
            _peer[event.peer]->clothing[items[state.id].cloth_type] = 0;
        else
            _peer[event.peer]->clothing[items[state.id].cloth_type] = state.id;

        /* update visuals. */
        clothing_visuals(event);
    }
    else {
        // @todo
    }
}
