/* child: packet.hpp */
/* parent: items.hpp */
#define DEBUG
#define ENET_IMPLEMENTATION
#include "include/enet.hpp"

#include <mutex>

#if defined(DEBUG)
#define LOG(message) std::clog << (message) << std::endl
#else
#define LOG(message)
#endif

class peer {
public:
    std::once_flag logging_in{}; /* without this, GT will keep pushing peer into the server. */
    std::once_flag sync_items{};
    signed netid{-1}; /* peer's netid is world identity. this will be useful for many packet sending */
    unsigned user_id{0}; /* peer's user_id is server identity. -> 5 CONNECTED peers in server, a new peer CONNECTS this value would be '6' (WONT CHANGE-> 1 person leaves, it's still 6.) */

    std::string visual_name{};
    std::vector<std::string> locked_worlds{}; /* this will only show worlds that is locked by a WORLD lock. not small/medium/big lock. */
    std::array<std::string, 5> recent_worlds{}; /* recent worlds, a list of 5 worlds, once it reaches 6 it'll be replaced by the oldest */

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
            fun(peer), peers.emplace_back(peer);
    return peers;
}

void read_peer(ENetPeer& p) 
{
    sqlite3* sql;
        sqlite3_open("peers.db", &sql);
    std::string peer_table = "\"" + (getp->tankIDName.empty() ? getp->requestedName : getp->tankIDName) + "\"";
{
    update_db(sql, std::format(
        "CREATE TABLE IF NOT EXISTS {0} (name {1}, password {1}, UNIQUE(name));", 
        peer_table, "TEXT NOT NULL"));
}
    update_db(sql, std::format("SELECT name, password FROM {};", peer_table), [](sqlite3_stmt* stmt){}, [&p](sqlite3_stmt* stmt) {
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
    std::string peer_table = "\"" + (getp->tankIDName.empty() ? getp->requestedName : getp->tankIDName) + "\"";
{
    update_db(sql, std::format(
        "CREATE TABLE IF NOT EXISTS {0} (name {1}, password {1}, UNIQUE(name));", 
        peer_table, "TEXT NOT NULL"));
}
    update_db(sql, std::format("INSERT OR REPLACE INTO {} (name, password) VALUES (?, ?);", peer_table), [&p](sqlite3_stmt* stmt) {
        sqlite3_bind_text(stmt, 1, getp->tankIDName.data(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, getp->tankIDPass.data(), -1, SQLITE_STATIC);
    });
    sqlite3_close(sql);
}
