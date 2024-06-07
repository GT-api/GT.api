/* child: packet.hpp */
/* parent: items.hpp */
#define DEBUG

#include <mutex>

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
    std::string_view requestedName{};
    std::string_view tankIDName{};
    std::string_view tankIDPass{};
    std::string_view country{};
    int protocol{};
    double game_version = {};
};

#define getpeer static_cast<peer*>(event.peer->data)

ENetHost* server;

std::vector<ENetPeer> peers(_ENetPeerState flag = ENET_PEER_STATE_CONNECTED) {
    std::vector<ENetPeer> peers{};
    for (ENetPeer& peer : std::ranges::subrange(server->peers, server->peers + server->peerCount)) 
        if (peer.state == flag)
            peers.emplace_back(peer);
    return peers;
}