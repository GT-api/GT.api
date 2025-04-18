
void input(ENetEvent event, const std::string& header)
{
    if (not create_rt(event, 1, 400)) return;
    std::string text{readch(std::string{header}, '|')[4]};
    {
        auto not_isspace = [](char c){ return not std::isspace(static_cast<unsigned char>(c)); };
        std::string::iterator start = std::ranges::find_if(text, not_isspace);
        std::string::iterator end = std::ranges::find_if(text.rbegin(), text.rend(), not_isspace).base();
        if (text.empty() or start >= end) return;
        text = std::string{start, end};
    }
    _peer[event.peer]->messages.push_back(steady_clock::now());
    if (_peer[event.peer]->messages.size() > 5) _peer[event.peer]->messages.pop_front();
    if (_peer[event.peer]->messages.size() == 5 and std::chrono::duration_cast<seconds>(steady_clock::now() - _peer[event.peer]->messages.front()).count() < 6)
        gt_packet(*event.peer, false, {
            "OnConsoleMessage", 
            "`6>>`4Spam detected! ``Please wait a bit before typing anything else.  "  
            "Please note, any form of bot/macro/auto-paste will get all your accounts banned, so don't do it!"
        });
    else if (text.starts_with('/')) 
    {
        action(*event.peer, "log", std::format("msg| `6{}``", text).c_str());
        if (cmd_pool.contains(text.substr(1, text.length())))
            (static_cast<void>(std::async(std::launch::async, cmd_pool[text.substr(1, text.length())], std::ref(event), std::move(text.substr(1, text.length())))));
        else action(*event.peer, "log", "msg|`4Unknown command.``  Enter `$/?`` for a list of valid commands.");
    }
    else if (text.back() == ' ' and text.length() > 1) text.pop_back(); // @note trim back spaces. "test "
    else if (text.front() == ' ' and text.length() > 1) text = text.substr(1, text.length() - 1); // @note trim front spacing. " test"
    else if (text.empty()) return; // @note ignore empty messages.
    else peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() and not _peer[event.peer]->recent_worlds.empty() and _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back())
            gt_packet(p, false, {
                "OnTalkBubble", 
                _peer[event.peer]->netid, 
                std::format("CP:0_PL:0_OID:_player_chat={}", text).c_str()
            }),
            gt_packet(p, false, {
                "OnConsoleMessage", 
                std::format("CP:0_PL:0_OID:_CT:[W]_ `6<`w{}``>`` `$`${}````", _peer[event.peer]->ltoken[0], text).c_str()
            });
    });
}
