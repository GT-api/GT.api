class slot {
    public:
    short id{0}; /* item id */
    short count{0}; /* the total amount of that item in that slot */
};

#include <mutex> /* std::once_flag */
#include <deque> /* std::deque */
#include <chrono>
using namespace std::chrono;

class peer {
public:
    std::once_flag logging_in{}; /* without this, GT will keep pushing peer into the server. */
    std::once_flag entered_game{}; /* only enter game once. this fixes many problems and to-be problems by exploiters */
    std::mutex post_enter{}; /* things that must be done when peer is in world, this value is reset once they leave. */

    signed netid{-1}; /* peer's netid is world identity. this will be useful for many packet sending */
    unsigned user_id{}; /* peer's user_id is server identity. -> 5 CONNECTED peers in server, a new peer CONNECTS this value would be '6' (WONT CHANGE-> 1 person leaves, it's still 6.) */

    std::array<float, 2> pos{}; /* x, y */
    bool facing_left{}; /* peer is directed towards the left direction */

    short slot_size{16}; /* amount of slots this peer has | were talking total slots not itemed slots, to get itemed slots do slot.size() */
    std::vector<slot> slots{{18, 1}, {32, 1}}; /* an array of each slot. storing id, count */

    std::vector<std::string> locked_worlds{}; /* this will only show worlds that is locked by a WORLD lock. not small/medium/big lock. */
    std::array<std::string, 5> recent_worlds{}; /* recent worlds, a list of 5 worlds, once it reaches 6 it'll be replaced by the oldest */

    std::array<steady_clock::time_point, 2> rate_limit{}; /* rate limit objects. for memory optimial reasons please manually increase array size. */
    std::deque<steady_clock::time_point> messages; /* last 5 que messages sent time, this is used to check for spamming */

    std::string nickname{};

    /* cached data from entering game; these values may not be changed */
    std::string requestedName{};
    std::string tankIDName{};

    std::string tankIDPass{};
    std::string country{};
};

#define getpeer static_cast<peer*>(event.peer->data)
#define getp static_cast<peer*>(p.data)

void register_peer(ENetEvent& event) {
    sqlite3* db;
    sqlite3_open("peer.db", &db);
    std::string table = "CREATE TABLE IF NOT EXISTS \"" + getpeer->tankIDName + "\" ("
                        "password TEXT);";
    sqlite3_exec(db, table.c_str(), nullptr, nullptr, nullptr);
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    std::string insert_sql = "INSERT INTO " + getpeer->tankIDName + " (password) VALUES (?);"; // Include password in your INSERT statement
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, getpeer->tankIDPass.c_str(), -1, nullptr); // Bind the password to the first parameter
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

bool read_peer(ENetEvent& event, const std::string& name) {
    sqlite3* db;
    sqlite3_open("peer.db", &db);
    std::string try_to = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + name + "';";
    sqlite3_stmt* check_stmt;
    if (sqlite3_prepare_v2(db, try_to.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_step(check_stmt) != SQLITE_ROW) {
        sqlite3_finalize(check_stmt);
        sqlite3_close(db);
        return false;
    }
    sqlite3_finalize(check_stmt);
    std::string select = "SELECT password FROM \"" + name + "\";";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, select.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* password = sqlite3_column_text(stmt, 0);
            if (password) {
                getpeer->tankIDPass = reinterpret_cast<const char*>(password);
            } else getpeer->tankIDPass = "";
        }
    }
    getpeer->tankIDName = name;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

/* @param pos please resize peer::rate_limit to fit the pos provided, understand the rules! if pos is 5, then size should be 6. */
template<typename length_T>
bool create_rt(ENetEvent& event, size_t pos, length_T length) 
{
    if (std::chrono::duration_cast<length_T>(std::chrono::steady_clock::now() - getpeer->rate_limit[pos]) <= length) return false;
    getpeer->rate_limit[pos] = std::chrono::steady_clock::now();
    return true;
}

#include <functional>
ENetHost* server;

std::vector<ENetPeer> peers(std::function<void(ENetPeer&)> fun = [](ENetPeer& peer){}) {
    std::vector<ENetPeer> peers{};
    for (ENetPeer& peer : std::ranges::subrange(server->peers, server->peers + server->peerCount)) 
        if (peer.state == ENET_PEER_STATE_CONNECTED)
            fun(peer), peers.emplace_back(peer);
    return peers;
}

class state {
    public:
	int type{};
    int netid{};
    int peer_state{};
    int id{};
    std::array<float, 2> pos{}; /* position */
    std::array<float, 2> speed{}; /* speed */
    std::array<int, 2> punch{}; /* punch pos (not range, but world pos) */
};

std::unique_ptr<state> get_state(const std::vector<std::byte>& packet) {
    auto s = std::make_unique<state>();
   s->type = *reinterpret_cast<const int*>(packet.data());
    s->netid = *reinterpret_cast<const int*>(packet.data() + 4);
    s->peer_state = *reinterpret_cast<const int*>(packet.data() + 12);
    /* unknown data */
    s->id = *reinterpret_cast<const int*>(packet.data() + 20);
    s->pos[0] = *reinterpret_cast<const float*>(packet.data() + 24);
    s->pos[1] = *reinterpret_cast<const float*>(packet.data() + 28);
    s->speed[0] = *reinterpret_cast<const float*>(packet.data() + 32);
    s->speed[1] = *reinterpret_cast<const float*>(packet.data() + 36);
    s->punch[0] = *reinterpret_cast<const int*>(packet.data() + 44);
    s->punch[1] = *reinterpret_cast<const int*>(packet.data() + 48);
    return s;
}

/* put it back into it's original form */
std::vector<std::byte> compress_state(const state& s)
{
    std::vector<std::byte> data(56, std::byte{0x00});
    *reinterpret_cast<int*>(data.data()) = s.type;
    *reinterpret_cast<int*>(data.data() + 4) = s.netid;
    *reinterpret_cast<int*>(data.data() + 12) = s.peer_state;
    /* unknown data */
    *reinterpret_cast<int*>(data.data() + 20) = s.id;
    *reinterpret_cast<float*>(data.data() + 24) = s.pos[0];
    *reinterpret_cast<float*>(data.data() + 28) = s.pos[1];
    *reinterpret_cast<float*>(data.data() + 32) = s.speed[0];
    *reinterpret_cast<float*>(data.data() + 36) = s.speed[1];
    *reinterpret_cast<int*>(data.data() + 44) = s.punch[0];
    *reinterpret_cast<int*>(data.data() + 48) = s.punch[1];
    return data;
}

void inventory_visuals(ENetPeer& p)
{
	int size = getp->slots.size();
    std::vector<std::byte> data(66 + (size * sizeof(int)) + sizeof(int), std::byte(0x0));
    std::array<int, 5> buffer{0x4, 0x9, -1, 0x0/* unknown data */, 0x8}; 
    *reinterpret_cast<std::array<int, 5>*>(data.data()) = buffer;
    *reinterpret_cast<int*>(data.data() + 66 - sizeof(int)) = _byteswap_ulong(size);
    *reinterpret_cast<int*>(data.data() + 66 - (2 * sizeof(int))) = _byteswap_ulong(getp->slot_size);
    for (int i = 0; i < size; ++i)
        *reinterpret_cast<int*>(data.data() + (i * sizeof(int)) + 66) = 
            ((static_cast<int>(getp->slots.at(i).id) bitor (static_cast<int>(getp->slots.at(i).count) << 16) bitand 0x00FFFFFF));
            
	enet_peer_send(&p, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}
