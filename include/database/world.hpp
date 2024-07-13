class block 
{
    public:
    short fg{0}, bg{0};
    std::array<int, 2> hits{0, 0}; /* fg, bg */ // -> stack object
};

class ifloat 
{
    public:
    int uid{0};
    short id{0};
    short count{0};
    float x, y;
};

class world {
    public:
    short x{100}, y{60};
    std::string name{};
    short visitors{0}; // -> stack object
    std::vector<block> blocks; /* all blocks, size of 1D meaning (6000) instead of (100, 60) */
    std::vector<ifloat> ifloats{}; /* floating items */
}; 
/* modify stack objects easily. these objects will remain in the stack not in world.db */
std::unordered_map<std::string, world> worlds{}; 

/* @brief push back a world in world.db */
void register_world(std::unique_ptr<world>& w) 
{
    sqlite3* db;
    sqlite3_open("world.db", &db);
    std::string table = "CREATE TABLE IF NOT EXISTS \"" + w->name + "\" ("
                        "id INTEGER PRIMARY KEY, "
                        "fg INTEGER, "
                        "bg INTEGER);";
    sqlite3_exec(db, table.c_str(), nullptr, nullptr, nullptr);
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    std::string insert_sql = "INSERT INTO " + w->name + " (id, fg, bg) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt, nullptr);
    for (size_t i = 0; i < w->blocks.size(); ++i) 
    {
        sqlite3_bind_int(stmt, 1, i);
        sqlite3_bind_int(stmt, 2, w->blocks[i].fg);
        sqlite3_bind_int(stmt, 3, w->blocks[i].bg);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/*
 @brief updates world.db for only 1 tile. this may save memory however the commit of opening world.db is already costly, please call this function sparingly
    NEVER use this function if your only updating worlds stack objects! these objects are not store-worthly
*/
void overwrite_tile(std::unique_ptr<world>& w, int blockID, const block& b) 
{
    sqlite3* db;
    sqlite3_open("world.db", &db);
    std::string update_sql = "UPDATE " + w->name + " SET fg = ?, bg = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, update_sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, b.fg);
    sqlite3_bind_int(stmt, 2, b.bg);
    sqlite3_bind_int(stmt, 3, blockID);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

std::unique_ptr<world> read_world(const std::string& name) 
{
    sqlite3* db;
    sqlite3_open("world.db", &db);
    std::string try_to = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + name + "';";
    sqlite3_stmt* check_stmt;
    if (sqlite3_prepare_v2(db, try_to.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) 
    {
        sqlite3_close(db);
        return nullptr;
    }

    if (sqlite3_step(check_stmt) != SQLITE_ROW) 
    {
        sqlite3_finalize(check_stmt);
        sqlite3_close(db);
        return nullptr;
    }
    sqlite3_finalize(check_stmt);
    std::string select = "SELECT fg, bg FROM \"" + name + "\";"; /* ignore id as it's just an index for writing */
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, select.c_str(), -1, &stmt, nullptr);
    std::unique_ptr<world> w = std::make_unique<world>();
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        short fg = sqlite3_column_int(stmt, 0);
        short bg = sqlite3_column_int(stmt, 1);
        w->blocks.push_back(block{fg, bg});
    }
    w->name = name;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return w;
}

void OnRequestWorldSelectMenu(ENetEvent event) 
{
    auto section = [](const auto& range, const auto& color) 
    {
        std::string result;
        for (const auto& name : range)
            if (not name.empty()) /* some may be stored empty but still an object. e.g. std::array */
                result += std::format("add_floater|{0}|0|0.5|{1}\n", name, color);
        if (not result.empty())
            result.pop_back(); 
        return result;
    };
    gt_packet(*event.peer, 1000, false, "OnRequestWorldSelectMenu", std::format(
        "add_filter|\nadd_heading|Top Worlds<ROW2>|{0}\nadd_heading|My Worlds<CR>|{1}\nadd_heading|Recently Visited Worlds<CR>|{2}",
        "", section(getpeer->locked_worlds, "2147418367"), section(getpeer->recent_worlds, "3417414143")).c_str());
    gt_packet(*event.peer, 0, false, "OnConsoleMessage", std::format("Where would you like to go? (`w{}`` online)", peers().size()).c_str());
}

void send_data(ENetPeer& peer, const std::vector<std::byte>& data)
{
    size_t size = data.size();
    unsigned four = 4;
    auto packet = enet_packet_create(nullptr, size + 5, ENET_PACKET_FLAG_RELIABLE);
    memcpy(packet->data, &four, sizeof(unsigned));
    memcpy(packet->data + 4, data.data(), size);
    if (static_cast<int>(data[12]) bitand 0x8)
        enet_packet_resize(packet, packet->dataLength + *std::bit_cast<DWORD*>(data.data() + 13)); /* resizes cause of data[12] -> peer_state */
    enet_peer_send(&peer, 0, packet);
}

void state_visuals(ENetEvent& event, state s) 
{
    s.netid = getpeer->netid;
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back()) 
            send_data(p, compress_state(s));
    });
}

void block_punched(ENetEvent& event, state s, int block1D) 
{
    worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 0 ?
    worlds[getpeer->recent_worlds.back()].blocks[block1D].hits[1]++ :
    worlds[getpeer->recent_worlds.back()].blocks[block1D].hits[0]++;
    s.type = 8; /* change packet type from 3 to 8. */
    s.id = 6; /* hit phase visuals */
	state_visuals(event, s);
}
