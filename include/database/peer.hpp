
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
    char ipv6[INET6_ADDRSTRLEN]; // modern-day IP for security. e.g. 2001:0db8:85a3:0000:0000:8a2e:0370:7334 -> https://en.wikipedia.org/wiki/IPv6_address

    signed netid{-1}; /* peer's netid is world identity. this will be useful for many packet sending */
    unsigned user_id{}; /* peer's user_id is server identity. -> 5 CONNECTED peers in server, a new peer CONNECTS this value would be '6' (WONT CHANGE-> 1 person leaves, it's still 6.) */

    std::array<float, 2> pos{}; /* x, y */
    bool facing_left{}; /* peer is directed towards the left direction */

    short slot_size{16}; /* amount of slots this peer has | were talking total slots not itemed slots, to get itemed slots do slot.size() */
    std::vector<slot> slots{{18, 1}, {32, 1}}; /* an array of each slot. storing id, count */

    std::vector<std::string> locked_worlds{}; /* this will only show worlds that is locked by a WORLD lock. not small/medium/big lock. */
    std::array<std::string, 5> recent_worlds{}; /* recent worlds, a list of 5 worlds, once it reaches 6 it'll be replaced by the oldest */
    std::string ongoing_world{}; /* the world the peer is inside. */

    std::array<steady_clock::time_point, 3> rate_limit{}; /* rate limit objects. for memory optimial reasons please manually increase array size. */
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

/* 
@param pos please resize peer::rate_limit to fit the pos provided, understand the rules! if pos is 5, then size should be 6. 
@return false if ratelimited
*/
template<typename length_T>
bool create_rt(ENetEvent& event, size_t pos, length_T length) 
{
    if (std::chrono::duration_cast<length_T>(std::chrono::steady_clock::now() - getpeer->rate_limit[pos]) <= length) return false;
    getpeer->rate_limit[pos] = std::chrono::steady_clock::now();
    return true;
}

#include <functional> // @note std::function<>
ENetHost* server;

std::vector<ENetPeer> peers(_ENetPeerState state = ENET_PEER_STATE_CONNECTED, std::function<void(ENetPeer&)> fun = [](ENetPeer& peer){}) {
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

state get_state(const std::vector<std::byte>& packet) {
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
    return *s;
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
