#include "pch.hpp"
#include "items.hpp"
#include "peer.hpp"
#include "network/packet.hpp"
#include "world.hpp"

#include <fstream>

world& world::read(std::string name)
{
    std::ifstream file(std::format("worlds\\{}.json", name));
    if (file.is_open()) 
    {
        nlohmann::json j;
        file >> j;
        this->name = name;
        this->owner = j.contains("owner") ? j["owner"].get<int>() : 00;
        for (const auto& i : j["bs"]) this->blocks.emplace_back(block{i["f"], i["b"], i.contains("l") ? i["l"].get<std::string>() : ""});
        for (const auto& i : j["fs"]) this->ifloats.emplace_back(ifloat{i["u"], i["i"], i["c"], std::array<float, 2>{i["p"][0], i["p"][1]}});
    }
    return *this;
}

world::~world() 
{
    if (!this->name.empty()) 
    {
        nlohmann::json j;
        if (this->owner != 00) j["owner"] = this->owner;
        for (const auto& [fg, bg, label, hits] : this->blocks) 
        {
            nlohmann::json list = {{"f", fg}, {"b", bg}};
            if (not label.empty()) list["l"] = label;
            j["bs"].push_back(list);
        }
        for (const auto& [uid, id, count, pos] : this->ifloats) 
            j["fs"].push_back({{"u", uid}, {"i", id}, {"c", count}, {"p", pos}});
        
        std::ofstream(std::format("worlds\\{}.json", this->name)) << j;
    }
}


std::unordered_map<std::string, world> worlds;

void send_data(ENetPeer& peer, const std::vector<std::byte>& data)
{
    std::size_t size = data.size();
    if (size < 14) return;
    ENetPacket *packet = enet_packet_create(nullptr, size + 5, ENET_PACKET_FLAG_RELIABLE);
    if (packet == nullptr || packet->dataLength < (size + 4)) return;
    packet->data[0] = { 04 };
    memcpy(packet->data + 4, data.data(), size); // @note for safety reasons I will not reinterpret the values.
    if (size >= 13 + sizeof(std::size_t)) 
    {
        std::size_t resize_forecast = *std::bit_cast<std::size_t*>(data.data() + 13); // @note we just wanna see if we can resize safely
        if (std::to_integer<unsigned char>(data[12]) & 0x8) // @note data[12] = peer_state in state class.
        {
            if (resize_forecast <= 512 && packet->dataLength + resize_forecast <= 512)
                enet_packet_resize(packet, packet->dataLength + resize_forecast);
        }
    }
    enet_peer_send(&peer, 1, packet);
}

void state_visuals(ENetEvent& event, state s) 
{
    s.netid = _peer[event.peer]->netid;
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() && _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()) 
            send_data(p, compress_state(s));
    });
}

void block_punched(ENetEvent& event, state s, block& b)
{
    (b.fg == 0) ? ++b.hits[1] : ++b.hits[0];
    s.type = 8; /* change packet type from 3 to 8. */
    s.id = 6; /* hit phase visuals */
	state_visuals(event, s);
}

void drop_visuals(ENetEvent& event, const std::array<short, 2>& im, const std::array<float, 2>& pos) 
{
    std::vector<ifloat>& ifloats{worlds[_peer[event.peer]->recent_worlds.back()].ifloats};
    ifloat it = ifloats.emplace_back(ifloat{ifloats.size() + 1, im[0], im[1], pos}); // @note a iterator ahead of time
    std::vector<std::byte> compress = compress_state({
        .type = 14, 
        .netid = -1, 
        .id = it.id, 
        .pos = {it.pos[0] * 32, it.pos[1] * 32}
    });
    *reinterpret_cast<int*>(&compress[8]) = it.uid;
    *reinterpret_cast<float*>(&compress[16]) = static_cast<float>(it.count);
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() && _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()) 
            send_data(p, compress);
    });
}

void clothing_visuals(ENetEvent &event) 
{
    gt_packet(*event.peer, true, {
        "OnSetClothing", 
        std::vector<float>{_peer[event.peer]->clothing[hair], _peer[event.peer]->clothing[shirt], _peer[event.peer]->clothing[legs]}, 
        std::vector<float>{_peer[event.peer]->clothing[feet], _peer[event.peer]->clothing[face], _peer[event.peer]->clothing[hand]}, 
        std::vector<float>{_peer[event.peer]->clothing[back], _peer[event.peer]->clothing[head], _peer[event.peer]->clothing[charm]}, 
        -1429995521,
        std::vector<float>{_peer[event.peer]->clothing[ances], 0.0f, 0.0f}
    });
}
