
void join_request(ENetEvent event, const std::string& header) 
{
    if (not create_rt(event, 2, 500ms)) 
    {
        gt_packet(*event.peer, 0, false, "OnFailedToEnterWorld");
        return;
    }
    std::string big_name{readpipe(std::string{header})[3]};
    if (not alpha(big_name) or big_name.empty())
    {
        gt_packet(*event.peer, 0, false, "OnConsoleMessage", "Sorry, spaces and special characters are not allowed in world or door names.  Try again.");
        gt_packet(*event.peer, 0, false, "OnFailedToEnterWorld");
        return;
    }
    std::ranges::transform(big_name, big_name.begin(), [](char c) { return std::toupper(c); });
    std::unique_ptr<world> w = read_world(big_name);
    if (w == nullptr) /* create a new world */
    {
        w = std::make_unique<world>(world{.name = big_name}); /* replace nullptr with world constructor */
        seed random{};
        auto main_door = random.fast(2, 100 * 60 / 100 - 4);
        std::vector<block> blocks(100 * 60, block{0, 0});
        for (auto& b : blocks) 
        {
            auto i = &b - &blocks[0];
            if (i >= 3700) 
                b.bg = 14, // cave background
                b.fg = (i >= 3800 and i < 5000 /* lava level */ and not random.fast(0, 38)) ? 10 : 
                    (i > 5000 and i < 5400 /* bedrock level */ and random.fast(0, 7) < 3) ? 4 : 
                    (i >= 5400) ? 8 : 2;
            if (i == 3600 + main_door) b.fg = 6; // main door
            if (i == 3700 + main_door) b.fg = 8; // bedrock below the main door
        }
        w->blocks = std::move(blocks);
        register_world(w);
    }
    getpeer->netid = ++w->visitors;
    short y = w->blocks.size() / 100, x = w->blocks.size() / y;
    std::vector<std::byte> data(85 + w->name.length() + (8 * w->blocks.size()) + 8 + (16 * w->ifloats.size()) + 1000/*TODO*/, std::byte{0x00});
    data[0] = std::byte{0x4};
    data[4] = std::byte{0x4};
    data[16] = std::byte{0x8};
    unsigned char len = w->name.length(); /* Growtopia limits world name length hence 255 is plenty of space */
    data[66] = std::byte{len};
    for (size_t i = 0; i < static_cast<int>(len); ++i)
        data[68 + i] = static_cast<std::byte>(w->name[i]);
    data[68 + static_cast<int>(len)] = static_cast<std::byte>(x);
    data[72 + static_cast<int>(len)] = static_cast<std::byte>(y);
    *reinterpret_cast<unsigned short*>(data.data() + 76 + len) = static_cast<unsigned short>(w->blocks.size());
    int pos = 85 + static_cast<int>(len);
    short i = 0;
    for (const auto& block : w->blocks)
    {
        auto [fg, bg, hits] = block;
        *reinterpret_cast<short*>(data.data() + pos) = fg;
        *reinterpret_cast<short*>(data.data() + (pos + 2)) = bg;
        if (fg == 6) /* TODO all doors, and custom bubbles (not only EXIT) */
        {
            getpeer->pos[0] = (i % x) * 32;
            getpeer->pos[1] = (i / x) * 32;
            data[pos + 8] = std::byte{0x1};
            *reinterpret_cast<short*>(data.data() + pos + 9) = 4;
            for (size_t i = 0; i < 4; ++i)
                data[pos + 11 + i] = static_cast<std::byte>("EXIT"[i]);
            pos += 8;
        }
        pos += 8;
        ++i;
    }
    *reinterpret_cast<int*>(data.data() + pos) = static_cast<int>(w->ifloats.size());
    *reinterpret_cast<int*>(data.data() + pos + 4) = static_cast<int>(w->ifloats.size());
    pos += 8;
    for (const auto& ifloat : w->ifloats) 
    {
        auto [id, count, uid, x, y] = ifloat;
        *reinterpret_cast<short*>(data.data() + pos) = id;
        *reinterpret_cast<float*>(data.data() + pos + 2) = x;
        *reinterpret_cast<float*>(data.data() + pos + 6) = y;
        *reinterpret_cast<short*>(data.data() + pos + 10) = count;
        *reinterpret_cast<int*>(data.data() + pos + 12) = uid;
        pos += 16;
    }
    enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
    for (size_t i = 0; i < getpeer->recent_worlds.size() - 1; ++i)
        getpeer->recent_worlds[i] = getpeer->recent_worlds[i + 1];
    getpeer->recent_worlds.back() = w->name;
    getpeer->ongoing_world = w->name;
    EmoticonDataChanged(event);
    gt_packet(*event.peer, 0, false, "OnSpawn", std::format("spawn|avatar\nnetID|{0}\nuserID|{1}\ncolrect|0|0|20|30\nposXY|{2}|{3}\nname|{4}\ncountry|{5}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\ntype|local\n",
        getpeer->netid, getpeer->user_id, static_cast<int>(getpeer->pos[0]), static_cast<int>(getpeer->pos[1]), getpeer->nickname, getpeer->country).c_str());
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back() and getp->user_id not_eq getpeer->user_id)
        {
            gt_packet(p, 0, false, "OnSpawn", std::format("spawn|avatar\nnetID|{0}\nuserID|{1}\ncolrect|0|0|20|30\nposXY|{2}|{3}\nname|{4}\ncountry|{5}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\n",
                getp->netid, getp->user_id, static_cast<int>(getp->pos[0]), static_cast<int>(getp->pos[1]), getp->nickname, getp->country).c_str());
            gt_packet(p, 0, false, "OnConsoleMessage", std::format("`5<`w{0}`` entered, `w{1}`` others here>``", 
                getpeer->nickname, w->visitors).c_str());
            gt_packet(p, 0, false, " OnTalkBubble", getpeer->netid, std::format("`5<`w{0}`` entered, `w{1}`` others here>``", 
                getpeer->nickname, w->visitors).c_str());
        }
    });
    gt_packet(*event.peer, 0, false, "OnConsoleMessage", std::format("World `w{0}`` entered.  There are `w{1}`` other people here, `w{2}`` online.",
        w->name, w->visitors - 1, peers().size()).c_str());
    inventory_visuals(*event.peer);
    if (worlds.find(w->name) == worlds.end()) /* so basically checks if world already on the stack else push back it (emplace) */
        worlds.emplace(w->name, *w);
}