
void quit_to_exit(ENetEvent& event, std::string& header) {
    std::unique_ptr<world> w = read_world(getpeer->recent_worlds.back());
    peers([&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back() and getp->user_id not_eq getpeer->user_id) 
        {
            gt_packet(p, 0, false, "OnConsoleMessage", std::format("`5<`w{0}`` left, `w{1}`` others here>``", getpeer->requestedName, --worlds[w->name].visitors).c_str());
            gt_packet(p, 0, true, "OnRemove", std::format("netID|{}\npId|\n", getpeer->netid).c_str());
        }
    });
    if (worlds[w->name].visitors <= 0)
        worlds.erase(w->name);
    getpeer->post_enter.unlock();
    OnRequestWorldSelectMenu(event);
}