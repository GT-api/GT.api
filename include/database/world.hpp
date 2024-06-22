class block {
    public:
    short fg{0}, bg{0};
    unsigned flags{0x00000000};
    std::string label;
};

class world {
    public:
    short x{100}, y{60};
    std::string name{};
    short visitors{0};
    std::vector<block> blocks;
};

void write_world(std::unique_ptr<world>& w) {
    sqlite3* db;
    sqlite3_open("world.db", &db);
    std::string table = "CREATE TABLE IF NOT EXISTS \"" + w->name + "\" ("
                        "fg INTEGER, "
                        "bg INTEGER);";
    sqlite3_exec(db, table.c_str(), nullptr, nullptr, nullptr);
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    std::string insert_sql = "INSERT INTO " + w->name + " (fg, bg) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt, nullptr);
    for (const auto& block : w->blocks) {
        sqlite3_bind_int(stmt, 1, block.fg);
        sqlite3_bind_int(stmt, 2, block.bg);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

std::unique_ptr<world> read_world(const std::string& name) {
    sqlite3* db;
    sqlite3_open("world.db", &db);
    std::string try_to = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + name + "';";
    sqlite3_stmt* check_stmt;
    if (sqlite3_prepare_v2(db, try_to.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return nullptr;
    }

    if (sqlite3_step(check_stmt) != SQLITE_ROW) {
        sqlite3_finalize(check_stmt);
        sqlite3_close(db);
        return nullptr;
    }
    sqlite3_finalize(check_stmt);
    std::string select = "SELECT fg, bg FROM \"" + name + "\";";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, select.c_str(), -1, &stmt, nullptr);
    std::unique_ptr<world> w = std::make_unique<world>();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        short fg = sqlite3_column_int(stmt, 0);
        short bg = sqlite3_column_int(stmt, 1);
        w->blocks.push_back(block{fg, bg});
    }
    w->name = name;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return w;
}

void OnRequestWorldSelectMenu(ENetEvent event) {
    auto section = [](const auto& range, const char* color) 
    {
        std::string result;
        for (const auto& name : range)
            if (not name.empty()) /* some may be stored empty but still an object. e.g. std::array */
                result += std::format("add_floater|{0}|0|0.5|{1}\n", name, color);
        if (not result.empty())
            result.pop_back(); 
        return result;
    };
    gt_packet(*event.peer, 0, "OnRequestWorldSelectMenu", std::format(
        "add_filter|\nadd_heading|Top Worlds<ROW2>|{0}\nadd_heading|My Worlds<CR>|{1}\nadd_heading|Recently Visited Worlds<CR>|{2}",
        "", section(getpeer->locked_worlds, "2147418367"), section(getpeer->recent_worlds, "3417414143")).c_str());
    gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("Where would you like to go? (`w{}`` online)", peers().size()).c_str());
}

void send_data(int a1, std::vector<std::byte> data, void *a4, ENetPeer& peer)
{
    size_t size = data.size();
    if (a1 == 4 and (static_cast<int>(data[12]) bitand 8))
    {
        auto packet = enet_packet_create(0, size + *reinterpret_cast<DWORD*>(data.data() + 13) + 5, ENET_PACKET_FLAG_RELIABLE);
        int four = 4;
        memcpy(packet->data, &four, 4);
        memcpy(packet->data + 4, data.data(), size);
        memcpy(packet->data + size + 4, a4, *reinterpret_cast<DWORD*>(data.data() + 13));
        enet_peer_send(&peer, 0, packet);
    }
    else
    {
        auto packet = enet_packet_create(0, size + 5, ENET_PACKET_FLAG_RELIABLE);
        memcpy(packet->data, &a1, 4);
        memcpy(packet->data + 4, data.data(), size);
        enet_peer_send(&peer, 0, packet);
    }
}

void state_visuals(ENetEvent& event, state s) {
    peers([&](ENetPeer& p) {
            if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back()) {
            s.netid = getpeer->netid;
            send_data(4, compress_state(s), 0, p);
        }
    });
}