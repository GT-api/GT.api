
void enter_game(ENetEvent event, const std::string& header) 
{
    std::call_once(getpeer->entered_game, [&]() 
    {
        getpeer->user_id = peers().size(); // @todo logic issue
        packet(*event.peer, std::format("action|log\nmsg|Welcome back, `w`w{}````.", getpeer->nickname = "guest").c_str());
        OnRequestWorldSelectMenu(event);
    });
}