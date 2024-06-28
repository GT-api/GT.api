
void enter_game(ENetEvent& event, std::string& header) 
{
    std::call_once(getpeer->entered_game, [&]() 
    {
        getpeer->user_id = peers().size();
        gt_packet(*event.peer, 0, true, "OnConsoleMessage", std::format("Welcome back, `w`w{}````.", getpeer->requestedName).c_str());
        OnRequestWorldSelectMenu(event);
    });
}