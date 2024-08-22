
std::unordered_map<std::string, std::function<void(ENetEvent&, const std::string&)>> cmd_pool
{
    {"help", [](ENetEvent& event, const std::string& text) 
    {
        packet(*event.peer, "action|log\nmsg|>> Commands: /wave /dance /love /sleep /facepalm /fp /smh /yes /no /omg /idk /shrug /furious /rolleyes /foldarms /stubborn /fold /dab /sassy /dance2 /march /grumpy /shy");
    }},
    {"wave", &Action}, {"dance", &Action}, {"love", &Action}, {"sleep", &Action}, {"facepalm", &Action}, {"fp", &Action}, {"smh", &Action}, {"yes", &Action}, 
    {"no", &Action}, {"omg", &Action}, {"idk", &Action}, {"shrug", &Action}, {"furious", &Action}, {"rolleyes", &Action}, {"foldarms", &Action}, {"fa", &Action}, 
    {"stubborn", &Action}, {"fold", &Action}, {"dab", &Action}, {"sassy", &Action}, {"dance2", &Action}, {"march", &Action}, {"grumpy", &Action}, {"shy", &Action}
};

void input(ENetEvent event, const std::string& header)
{
    if (not create_rt(event, 1, 400ms)) return;
    std::string text{readpipe(std::string{header})[4]};
    getpeer->messages.push_back(steady_clock::now());
    if (getpeer->messages.size() > 5) getpeer->messages.pop_front();
    if (getpeer->messages.size() == 5 and std::chrono::duration_cast<seconds>(steady_clock::now() - getpeer->messages.front()).count() < 6)
        gt_packet(*event.peer, false, "OnConsoleMessage", 
        "`6>>`4Spam detected! ``Please wait a bit before typing anything else.  Please note, any form of bot/macro/auto-paste will get all your accounts banned, so don't do it!");
    else if (text.starts_with('/')) 
    {
        packet(*event.peer, std::format("action|log\nmsg| `6{}``", text).c_str());
        if (cmd_pool.contains(text.substr(1, text.size())))
            (static_cast<void>(std::async(std::launch::async, cmd_pool[text.substr(1, text.size())], std::ref(event), std::move(text.substr(1, text.size())))));
        else packet(*event.peer, "action|log\nmsg|`4Unknown command.``  Enter `$/?`` for a list of valid commands.");
    }
    else peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back())
            gt_packet(p, false, "OnTalkBubble", getpeer->netid, std::format("CP:0_PL:0_OID:_player_chat={}", text).c_str()),
            gt_packet(p, false, "OnConsoleMessage", std::format("CP:0_PL:0_OID:_CT:[W]_ `6<`w{}``>`` `$`${}````", getpeer->nickname, text).c_str());
    });
}
