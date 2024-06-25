/* child: packet.hpp */
/* parent: items.hpp */

class slot {
    public:
    short id{0}; /* item id */
    short count{0}; /* the total amount of that item in that slot */
};

#include <mutex> /* std::once_flag */

class peer {
public:
    std::once_flag logging_in{}; /* without this, GT will keep pushing peer into the server. */
    std::once_flag entered_game{}; /* only enter game once. this fixes many problems and to-be problems by exploiters */
    signed netid{-1}; /* peer's netid is world identity. this will be useful for many packet sending */
    unsigned user_id{0}; /* peer's user_id is server identity. -> 5 CONNECTED peers in server, a new peer CONNECTS this value would be '6' (WONT CHANGE-> 1 person leaves, it's still 6.) */
    std::array<float, 2> pos{0.00, 0.00};

    short slot_size{16}; /* amount of slots this peer has | were talking total slots not itemed slots, to get itemed slots do slot.size() */
    std::vector<slot> slots{}; /* an inventory slot starting at slot[0], don't know C++? just do slots.emplace_back to push a new item inside inventory */

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
    memcpy(&s->type, packet.data(), sizeof(int));
    memcpy(&s->netid, packet.data() + 4, sizeof(int));
    memcpy(&s->peer_state, packet.data() + 12, sizeof(int));
    memcpy(&s->id, packet.data() + 20, sizeof(int));
    memcpy(&s->pos[0], packet.data() + 24, sizeof(float));
    memcpy(&s->pos[1], packet.data() + 28, sizeof(float));
    memcpy(&s->speed[0], packet.data() + 32, sizeof(float));
    memcpy(&s->speed[1], packet.data() + 36, sizeof(float));
    memcpy(&s->punch[0], packet.data() + 44, sizeof(int));
    memcpy(&s->punch[1], packet.data() + 48, sizeof(int));
    return s;
}

/* put it back into it's original form */
std::vector<std::byte> compress_state(const state& s)
{
    std::vector<std::byte> data(56, std::byte{0x00});
    memcpy(data.data(), &s.type, sizeof(int));
    memcpy(data.data() + 4, &s.netid, sizeof(int));
    memcpy(data.data() + 12, &s.peer_state, sizeof(int));
    memcpy(data.data() + 20, &s.id, sizeof(int));
    memcpy(data.data() + 24, &s.pos[0], sizeof(float));
    memcpy(data.data() + 28, &s.pos[1], sizeof(float));
    memcpy(data.data() + 32, &s.speed[0], sizeof(float));
    memcpy(data.data() + 36, &s.speed[1], sizeof(float));
    memcpy(data.data() + 44, &s.punch[0], sizeof(float));
    memcpy(data.data() + 48, &s.punch[1], sizeof(float));
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
