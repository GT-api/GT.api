#include "items.hpp"
#include "peer.hpp"

#include <ranges> // @note std::ranges::subrange

std::unordered_map<ENetPeer*, std::shared_ptr<peer>> _peer;

bool create_rt(ENetEvent& event, std::size_t pos, int64_t length)
{
    auto now = std::chrono::steady_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _peer[event.peer]->rate_limit[pos]);

    if (time_elapsed.count() <= length) 
        return false;

    _peer[event.peer]->rate_limit[pos] = now;
    return true;
}

ENetHost* server;

std::vector<ENetPeer> peers(_ENetPeerState state, std::function<void(ENetPeer&)> fun) {
    std::vector<ENetPeer> peers{};
    for (ENetPeer& peer : std::ranges::subrange(server->peers, server->peers + server->peerCount)) 
        if (peer.state == ENET_PEER_STATE_CONNECTED)
            fun(peer), peers.emplace_back(peer);
    return peers;
}

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

void inventory_visuals(ENetEvent &event)
{
	int size = _peer[event.peer]->slots.size();
    std::vector<std::byte> data(66 + (size * sizeof(int)) + sizeof(int), std::byte(0x0));
    *reinterpret_cast<std::array<int, 5>*>(&data[0]) = {0x4, 0x9, -1, 0x0, 0x8};
    *reinterpret_cast<unsigned long*>(&data[62]) = _byteswap_ulong(size); // @note 66....
    *reinterpret_cast<unsigned long*>(&data[58]) = _byteswap_ulong(_peer[event.peer]->slot_size); // @note 62....
    for (int i = 0; i < size; ++i)
        *reinterpret_cast<int*>(&data[(i * sizeof(int)) + 66]) = 
            (static_cast<int>(_peer[event.peer]->slots.at(i).id) bitor (static_cast<int>(_peer[event.peer]->slots.at(i).count) << 16) bitand 0x00FFFFFF);
            
	enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}
