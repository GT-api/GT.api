
void enter_game(ENetEvent event, const std::string& header) 
{
    std::call_once(_peer[event.peer]->entered_game, [&]() 
    {
        _peer[event.peer]->user_id = peers().size(); // @todo logic issue
        OnRequestWorldSelectMenu(event);
    });
}