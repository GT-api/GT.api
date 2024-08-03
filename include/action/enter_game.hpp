
void enter_game(ENetEvent event, const std::string& header) 
{
    std::call_once(getpeer->entered_game, [&]() 
    {
        getpeer->user_id = peers().size(); // @todo logic issue
        gt_packet(*event.peer, 0, false, "SetHasGrowID", getpeer->login[0].empty() ? 0 : 1, getpeer->login[0].c_str(), getpeer->login[1].c_str());
        packet(*event.peer, std::format("action|log\nmsg|Welcome back, `w`w{}````.", getpeer->nickname = "guest").c_str());
        OnRequestWorldSelectMenu(event);
        getpeer->slots.emplace_back(2, 200);
        getpeer->slots.emplace_back(14, 200);
    });
}