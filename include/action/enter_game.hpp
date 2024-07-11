
void enter_game(ENetEvent& event, const std::string& header) 
{
    std::call_once(getpeer->entered_game, [&]() 
    {
        getpeer->user_id = peers().size();
        getpeer->nickname = (getpeer->tankIDName.empty()) ? getpeer->requestedName : getpeer->tankIDName;
        gt_packet(*event.peer, 0, true, "OnConsoleMessage", std::format("Welcome back, `w`w{}````.", getpeer->nickname).c_str());
        OnRequestWorldSelectMenu(event);
        getpeer->slots.emplace_back(2, 200);
        getpeer->slots.emplace_back(14, 200);
    });
}