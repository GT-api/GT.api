
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
    std::array<float, 2> rest_pos{}; /* respawn position AKA main door: x, y */
    bool facing_left{}; /* peer is directed towards the left direction */

    short slot_size{16}; /* amount of slots this peer has | were talking total slots not itemed slots, to get itemed slots do slot.size() */
    std::vector<slot> slots{{18, 1}, {32, 1}}; /* an array of each slot. storing id, count */
    /* set slot::count to nagative value if you want to remove an amount. */
    void emplace(slot s) 
    {
        if (auto it = std::find_if(slots.begin(), slots.end(), [&](const auto& found) { return found.id == s.id; }); it not_eq slots.end()) 
            it->count = std::clamp(it->count + s.count, 0, 200); // @note keeps the count within 200.
        else slots.emplace_back(std::move(s)); // @note no such item in inventory, so we create a new entry.
    }

    std::vector<std::string> locked_worlds{}; /* this will only show worlds that is locked by a WORLD lock. not small/medium/big lock. */
    std::array<std::string, 5> recent_worlds{}; /* recent worlds, a list of 5 worlds, once it reaches 6 it'll be replaced by the oldest */
    std::string ongoing_world{}; /* the world the peer is inside. */

    std::array<steady_clock::time_point, 3> rate_limit{}; /* rate limit objects. for memory optimial reasons please manually increase array size. */
    std::deque<steady_clock::time_point> messages; /* last 5 que messages sent time, this is used to check for spamming */

    const char* nickname{}; // @note peer's displayed name. this is only used in packets hence it is a C-type container
    std::string country{}; // @note country initials e.g. us, id, jp, uk
};

#define getpeer static_cast<peer*>(event.peer->data)
#define getp static_cast<peer*>(p.data)

/* 
@param pos please resize peer::rate_limit to fit the pos provided, understand the rules! if pos is 5, then size should be 6. 
@return false if ratelimited
*/
template<typename length_T>
bool create_rt(ENetEvent& event, std::size_t pos, length_T length) 
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
    // @todo unknown data
    int peer_state{};
    // @todo unknown data
    int id{}; // @note peer's active hand, so 18 (fist) = punching, 32 (wrench) interacting, ect...
    std::array<float, 2> pos{}; // @note position {x, y}
    std::array<float, 2> speed{}; // @note player movement effect (gravity, speed, ect) {x, y}
    std::array<int, 2> punch{}; // @note punching/placing position {x, y}
};

state get_state(const std::vector<std::byte>& packet) {
    return state{
        .type = std::bit_cast<int*>(packet.data())[0],
        .netid = std::bit_cast<int*>(packet.data())[1],
        .peer_state = std::bit_cast<int*>(packet.data())[3],
        .id = std::bit_cast<int*>(packet.data())[5],
        .pos = {std::bit_cast<float*>(packet.data())[6], std::bit_cast<float*>(packet.data())[7]},
        .speed = {std::bit_cast<float*>(packet.data())[8], std::bit_cast<float*>(packet.data())[9]},
        .punch = {std::bit_cast<int*>(packet.data())[11], std::bit_cast<int*>(packet.data())[12]}
    };
}

/* put it back into it's original form */
std::vector<std::byte> compress_state(const state& s) {
    std::vector<std::byte> data(56, std::byte{0x00});
    std::bit_cast<int*>(data.data())[0] = s.type;
    std::bit_cast<int*>(data.data())[1] = s.netid;
    std::bit_cast<int*>(data.data())[3] = s.peer_state;
    std::bit_cast<int*>(data.data())[5] = s.id;
    std::bit_cast<float*>(data.data())[6] = s.pos[0];
    std::bit_cast<float*>(data.data())[7] = s.pos[1];
    std::bit_cast<float*>(data.data())[8] = s.speed[0];
    std::bit_cast<float*>(data.data())[9] = s.speed[1];
    std::bit_cast<int*>(data.data())[11] = s.punch[0];
    std::bit_cast<int*>(data.data())[12] = s.punch[1];
    return data;
}

void inventory_visuals(ENetPeer& p)
{
	int size = getp->slots.size();
    std::vector<std::byte> data(66 + (size * sizeof(int)) + sizeof(int), std::byte(0x0));
    *reinterpret_cast<std::array<int, 5>*>(&data[0]) = {0x4, 0x9, -1, 0x0, 0x8};
    *reinterpret_cast<unsigned long*>(&data[62]) = _byteswap_ulong(size); // @note 66....
    *reinterpret_cast<unsigned long*>(&data[58]) = _byteswap_ulong(getp->slot_size); // @note 62....
    for (int i = 0; i < size; ++i)
        *reinterpret_cast<int*>(&data[(i * sizeof(int)) + 66]) = 
            ((static_cast<int>(getp->slots.at(i).id) bitor (static_cast<int>(getp->slots.at(i).count) << 16) bitand 0x00FFFFFF));
            
	enet_peer_send(&p, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}
