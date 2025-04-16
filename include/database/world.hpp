#pragma once

/* fg, bg, hits */
class block 
{
    public:
    short fg{0}, bg{0};
    std::array<int, 2> hits{0, 0}; /* fg, bg */
};

/* uid, id, count, pos*/
class ifloat 
{
    public:
    std::size_t uid{0};
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
            try 
            {
                file >> j;
            } 
            catch (const nlohmann::json::parse_error& e) 
            {
                printf("%s", e.what());
                return *this;
            }
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
extern std::unordered_map<std::string, world> worlds;

void send_data(ENetPeer& peer, const std::vector<std::byte>& data);

void state_visuals(ENetEvent& event, state s);

void block_punched(ENetEvent& event, state s, const int block1D);

void drop_visuals(ENetEvent& event, const std::array<short, 2>& im, const std::array<float, 2>& pos);

void clothing_visuals(ENetEvent &event);
