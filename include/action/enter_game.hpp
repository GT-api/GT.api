
void enter_game(ENetEvent& event, const std::string header) 
{
    std::call_once(getpeer->entered_game, [&]() 
    {
        getpeer->nickname = (getpeer->tankIDName.empty()) ? getpeer->requestedName : getpeer->tankIDName;
        gt_packet(*event.peer, 0, true, "SetHasGrowID", getpeer->tankIDName.empty() ? 0 : 1, getpeer->tankIDName.c_str(), getpeer->tankIDPass.c_str());
        gt_packet(*event.peer, 0, true, "OnConsoleMessage", std::format("Welcome back, `w`w{}````.", getpeer->nickname).c_str());
        OnRequestWorldSelectMenu(event);
        getpeer->slots.emplace_back(2, 200);
        getpeer->slots.emplace_back(14, 200);
    });
}