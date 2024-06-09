/* child: packet.hpp */
/* parent: items.hpp */
#define DEBUG

#include <mutex>
#include <functional>

#if defined(DEBUG)
#define LOG(message) std::clog << (message) << std::endl
#else
#define LOG(message)
#endif

class peer {
public:
    std::once_flag logging_in{}; /* without this, GT will keep pushing peer into the server. */
    signed netid{-1}; /* peer's netid is world identity. this will be useful for many packet sending */
    unsigned user_id{0}; /* peer's user_id is server identity. -> 5 CONNECTED peers in server, a new peer CONNECTS this value would be '6' (WONT CHANGE-> 1 person leaves, it's still 6.) */
    unsigned gems{0}; /* gem count. due to being unsigned it will not bug out to nagative values, however 'if (gems > INT_MAX)' should be utilized */
    std::string visual_name{}; /* the name of peer when entering worlds, or displayed in broadcasts, chat box, ect. */

    /* cached data from entering game; these values may not be changed */
    std::string requestedName{};
    std::string tankIDName{};
    std::string tankIDPass{};
    std::string country{};
};

#define getpeer static_cast<peer*>(event.peer->data)
#define getp static_cast<peer*>(p.data)

ENetHost* server;

std::vector<ENetPeer> peers(std::function<void(ENetPeer&)> fun = [](ENetPeer& peer){}) {
    std::vector<ENetPeer> peers{};
    for (ENetPeer& peer : std::ranges::subrange(server->peers, server->peers + server->peerCount)) 
        if (peer.state == ENET_PEER_STATE_CONNECTED)
            fun(peer);
    return peers;
}

/*
@param cmd the 'command' that will be performed. e.g. CREATE, SELECT, INSERT -> create (table), read, modify
@param before actions before the data gets written
@param after actions after the data was written
*/
void update_db(sqlite3* sql, const std::string& cmd, 
std::function<void(sqlite3_stmt*)> before = [](sqlite3_stmt* stmt){},
std::function<void(sqlite3_stmt*)> after = [](sqlite3_stmt* stmt){}) 
{
    sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(sql, cmd.c_str(), -1, &stmt, nullptr) not_eq SQLITE_OK)
            std::cerr << std::format("sqlite3_prepare_v2() warning: {}\n", sqlite3_errmsg(sql));
        else before(stmt);
        if (sqlite3_step(stmt) not_eq SQLITE_DONE)
            std::cerr << std::format("sqlite3_step() warning: {}\n", sqlite3_errmsg(sql));
        else after(stmt);
        sqlite3_finalize(stmt);
}

void read_peer(ENetPeer& p) 
{
   sqlite3* sql;
    if (sqlite3_open("peers.db", &sql) != SQLITE_OK)
        std::cerr << "sqlite3_open() warning: " << sqlite3_errmsg(sql) << std::endl;
{
   std::string cmd = std::format(
        "CREATE TABLE IF NOT EXISTS {0} (name TEXT NOT NULL, password TEXT NOT NULL);", 
        getp->tankIDName.empty() ? getp->requestedName : getp->tankIDName);
    update_db(sql, cmd);
}
    std::string cmd = std::format(
        "SELECT name, password FROM {0};", getp->tankIDName.empty() ? getp->requestedName : getp->tankIDName);
    update_db(sql, cmd, [](sqlite3_stmt* stmt){}, [&p](sqlite3_stmt* stmt) {
        auto temp_peer = std::make_unique<peer>();
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            temp_peer->tankIDName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            temp_peer->tankIDPass = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        }
        p.data = temp_peer.release();
    });
    sqlite3_close(sql);
}

void write_peer(ENetPeer& p)
{
    sqlite3* sql;
        sqlite3_open("peers.db", &sql);
{
   std::string cmd = std::format(
        "CREATE TABLE IF NOT EXISTS {0} (name TEXT NOT NULL, password TEXT NOT NULL);", 
        getp->tankIDName.empty() ? getp->requestedName : getp->tankIDName);
     update_db(sql, cmd);
}
    std::string cmd = std::format(
        "INSERT INTO {0} (name, password) VALUES (?, ?);", 
        getp->tankIDName.empty() ? getp->requestedName : getp->tankIDName);
    update_db(sql, cmd, [&p](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, getp->tankIDName.data(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, getp->tankIDPass.data(), -1, SQLITE_STATIC);
    });
    sqlite3_close(sql);
}
