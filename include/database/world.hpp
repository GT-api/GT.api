
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
    std::array<float, 2> pos;
};

#include <fstream> // @note i/o fstream for reading/writing files
#include "nlohmann\json.hpp" // @note nlohmann::json for writing/reading JSON format

class world 
{
public:
    world& read(std::string name)
    {
        std::ifstream file(std::format("worlds\\{}.json", name));
        if (file.is_open()) 
        {
            nlohmann::json j;
            file >> j;
            this->name = name;
            for (const auto& i : j["bs"]) blocks.emplace_back(block{i["f"], i["b"]});
            for (const auto& i : j["fs"]) ifloats.emplace_back(ifloat{i["u"], i["i"], i["c"], std::array<float, 2>{i["p"][0], i["p"][1]}});
        }
        return *this;
    }
    std::string name{};
    short visitors{0}; // -> stack object
    std::vector<block> blocks; /* all blocks, size of 1D meaning (6000) instead of (100, 60) */
    std::vector<ifloat> ifloats{}; /* (i)tem floating */
    ~world() 
    {
        if (not this->name.empty())
        {
            nlohmann::json j;
            for (const auto& [fg, bg, hits] : blocks) j["bs"].push_back({{"f", fg}, {"b", bg}});
            for (const auto& [uid, id, count, pos] : ifloats) j["fs"].push_back({{"u", uid}, {"i", id}, {"c", count}, {"p", pos}});
            std::ofstream(std::format("worlds\\{}.json", this->name)) << j;
        }
    }
};
std::unordered_map<std::string, world> worlds{}; // @note remove in future commit...

void OnRequestWorldSelectMenu(ENetEvent event) 
{
    auto section = [](const auto& range, const auto& color) 
    {
        std::string result;
        for (const auto& name : range)
            if (not name.empty()) /* some may be stored empty but still an object. e.g. std::array */
                result += std::format("\nadd_floater|{0}|0|0.5|{1}", name, color);
        return result;
    };
    gt_packet(*event.peer, false, "OnRequestWorldSelectMenu", std::format(
        "add_filter|\nadd_heading|Top Worlds<ROW2>|{0}\nadd_heading|My Worlds<CR>|{1}\nadd_heading|Recently Visited Worlds<CR>|{2}\n",
        "\nadd_floater|wotd_world|\u013B WOTD|0|0.5|3529161471", section(getpeer->locked_worlds, "2147418367"), section(getpeer->recent_worlds, "3417414143")).c_str(), 0);
    gt_packet(*event.peer, false, "OnConsoleMessage", std::format("Where would you like to go? (`w{}`` online)", peers().size()).c_str());
}

void send_data(ENetPeer& peer, const std::vector<std::byte>& data)
{
    size_t size = data.size();
    if (size < 14) return;
    auto packet = enet_packet_create(nullptr, size + 5, ENET_PACKET_FLAG_RELIABLE);
    *reinterpret_cast<std::array<enet_uint8, 4>*>(packet->data) = {0x4};
    memcpy(packet->data + 4, data.data(), size); // @note for safety reasons I will not reinterpret the values.
    if (static_cast<int>(data[12]) bitand 0x8) // @note data[12] = peer_state in state class.
    {
        size_t resize_forecast = *std::bit_cast<size_t*>(data.data() + 13); // @note we just wanna see if we can resize safely
        if (packet->dataLength + resize_forecast <= size_t{512})
            enet_packet_resize(packet, packet->dataLength + resize_forecast);
    }
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

void drop_visuals(ENetEvent& event, short id, short count) 
{
    std::vector<ifloat>& ifloats{worlds[getpeer->recent_worlds.back()].ifloats};
    float x_nabor = (getpeer->facing_left ? getpeer->pos[0] - 1 : getpeer->pos[0] + 1); // @note get the x naboring tile of peer's position. Oー
    std::array<float, 2> nabor_pos = {x_nabor, getpeer->pos[1]}; // @note getpeer->pos but [0] is the naboring tile. O|
    ifloat it = ifloats.emplace_back(ifloat{static_cast<int>(ifloats.size()), id, count, nabor_pos}); // @note a iterator ahead of time
    std::vector<std::byte> compress = compress_state({.type = 14, .netid = -1, .id = it.id, .pos = {it.pos[0] * 32, it.pos[1] * 32}});
    *reinterpret_cast<int*>(compress.data() + 8) = it.uid;
    *reinterpret_cast<float*>(compress.data() + 16) = static_cast<float>(it.count);
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back()) 
            send_data(p, compress);
    });
}