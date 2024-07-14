
void quit_to_exit(ENetEvent& event, const std::string& header) 
{
    if (getpeer->post_world.empty()) return; // as stated on below comment. this must be false if we wanna create/read a ratelimit.
        if (not create_rt(event, 2, 500ms)) return; // stand-alone to the arguement above. OMG sorry if this is complex- basically this is a twin ratelimit to enter and/or leaving a world.
    worlds[getpeer->recent_worlds.back()].visitors--;
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back() and getp->user_id not_eq getpeer->user_id) 
        {
            gt_packet(p, 0, false, "OnConsoleMessage", std::format("`5<`w{0}`` left, `w{1}`` others here>``", getpeer->nickname, worlds[getpeer->recent_worlds.back()].visitors).c_str());
            gt_packet(p, 0, true, "OnRemove", std::format("netID|{}\npId|\n", getpeer->netid).c_str());
        }
    });
    if (worlds[getpeer->recent_worlds.back()].visitors <= 0)
        worlds.erase(getpeer->recent_worlds.back());
    OnRequestWorldSelectMenu(event);
    getpeer->post_enter.unlock();
    getpeer->post_world.clear();
    getpeer->netid = -1; // this will fix any packets being sent outside of world
}