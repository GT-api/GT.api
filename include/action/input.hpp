
void input(ENetEvent& event, std::string& header)
{
    getpeer->rate_limit[1] = steady_clock::now();
    std::string text{readpipe(std::string{header})[4]};
    if (text.starts_with(" ") or text.starts_with("/")) return; // TODO add commands
    getpeer->messages.push_back(steady_clock::now());
    if (getpeer->messages.size() > 5) getpeer->messages.pop_front();
    if (getpeer->messages.size() == 5 and std::chrono::duration_cast<seconds>(steady_clock::now() - getpeer->messages.front()).count() < 6)
    {
        gt_packet(*event.peer, 0, false, "OnConsoleMessage", "`6>>`4Spam detected! ``Please wait a bit before typing anything else.  Please note, any form of bot/macro/auto-paste will get all your accounts banned, so don't do it!");
        return;
    }
    peers([&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back())
        {
            gt_packet(p, 0, false, "OnTalkBubble", getpeer->netid, std::format("CP:0_PL:0_OID:_player_chat={}", text).c_str());
            gt_packet(p, 0, false, "OnConsoleMessage", std::format("CP:0_PL:0_OID:_CT:[W]_ `6<`w{0}``>`` `$`${1}````", getpeer->requestedName, text).c_str());
        }
    });
}