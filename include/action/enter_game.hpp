
void enter_game(ENetEvent event, const std::string& header) 
{
    std::call_once(_peer[event.peer]->entered_game, [&]() 
    {
        _peer[event.peer]->user_id = peers().size(); // @todo set user_id based on peer database count. e.g. 10 peer files: 10 + 1 (push back)

        OnRequestWorldSelectMenu(event); // @todo add packet delay

        gt_packet(*event.peer, false, {"OnConsoleMessage", std::format("Welcome back, `w`w{}````.", _peer[event.peer]->ltoken[0]).c_str()}); 
        gt_packet(*event.peer, false, {"SetHasGrowID", 1, _peer[event.peer]->ltoken[0], ""}); 
    });
}