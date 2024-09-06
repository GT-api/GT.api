
void Action(ENetEvent& event, const std::string& text) 
{
    std::string to_slang = (text == "facepalm") ? "fp" : (text == "shrug") ? "idk" : (text == "foldarms") ? "fold" : (text == "fa") ? "fold" : (text == "stubborn") ? "fold" : text;
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() and not _peer[event.peer]->recent_worlds.empty() and _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back())
            gt_packet(p, true, "OnAction", std::string("/" + to_slang).c_str());
    });
}
