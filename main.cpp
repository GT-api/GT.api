#include <iostream>
#include <fstream>
#include <format> /* enhanced string arguements & better logging */
#include <thread> /* std::jthread() & std::chrono / std::this_thread */
#include <vector>
#include <numeric> /* accumulate for items.dat hash (acc for short) */
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <random> /* for random generator */

#define ENET_IMPLEMENTATION
#include "include/enet.hpp"

#include "items.hpp"
#include "peer.hpp"
#include "packet.hpp"
#include "world.hpp"
#include "include/compress.hpp" /* for growtopia packet (byte) compressor; a must for a 'GTPS' */

using namespace std::literals;

/* 
@param str the whole content
@brief reads a string and seperates the pipes '|' and stores those pieces of info into a vector.
@return std::vector<std::string> of all the seperations of the pipes '|' e.g. Name|myName -> readpipe[1] = myName
*/
std::vector<std::string> readpipe(const std::string& str) {
    std::vector<std::string> separations;
    for (auto&& part : str | std::views::split('|')) {
        separations.emplace_back(std::string(std::ranges::begin(part), std::ranges::end(part)));
    }
    return separations;
}

int rand(const int min, const int max) {
    static std::random_device device;
    static std::mt19937 mt(device());
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(mt);
}

int main() {
    if (enet_initialize() not_eq 0) 
        std::cerr << "enet_initialize() failed" << std::endl, std::cin.ignore();

    ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};
    
    server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
              std::streamsize im_size = file.tellg();
                   if (im_size == -1) std::cerr << "failed to read items.dat in binary mode" << std::endl, std::cin.ignore();
        im_data.resize(im_size + 60);
        std::fill(im_data.begin(), im_data.begin() + 60, std::byte{0x0});
        std::array<int, 4> buffer{0x4, 0x10, -1, 0x8};
             for (size_t i = 0; i < buffer.size(); ++i) 
                 new (&im_data[i * sizeof(int)]) int(buffer[i]);
         new (&im_data[56]) size_t(im_size);
         file.seekg(0, std::ios::beg);
         file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
          file.close();
          auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
              hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                  [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
             LOG(std::format("items.dat hash: {0}", hash));
    }
    if (cache_items())
        LOG(std::format("cached {0} items from items.dat", items.size()));
    else LOG("failed to cache items from items.dat");

    ENetEvent event{};
    while (true) 
    {
        while (enet_host_service(server, &event, 1000) > 0)
            std::jthread([&](std::stop_token stop)
	        {
                LOG(std::format("event.type = {0}", (int)event.type));
                switch (event.type) 
                {
                    case ENET_EVENT_TYPE_NONE: break;
				    case ENET_EVENT_TYPE_CONNECT: 
                    {
                        if (enet_peer_send(event.peer, 0, enet_packet_create((const enet_uint8[]){0x1, 0x0, 0x0, 0x0}, 4, ENET_PACKET_FLAG_RELIABLE)) < 0) break;
                        event.peer->data = new peer{};
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT: 
                    {
                        delete getpeer;
                        event.peer->data = nullptr;
                        break;
                    }
                    case ENET_EVENT_TYPE_RECEIVE: 
                    {
                        LOG(std::format("event.packet->data = {0}", std::span{event.packet->data, event.packet->dataLength}[0]));
                        std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                            std::string header{packet.begin() + 4, packet.end() - 1};
                            LOG(header);
                        switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                        {
                            case 2: 
                            {
                                std::call_once(getpeer->logging_in, [&](){
                                    short offset{};
                                    std::ranges::replace(header, '\n', '|');
                                    readpipe(std::string{header})[0] == "requestedName" ? 
                                        getpeer->requestedName = readpipe(std::string{header})[1] : 
                                        getpeer->tankIDName = readpipe(std::string{header})[1], getpeer->tankIDPass = readpipe(std::string{header})[3];
                                    if (readpipe(std::string{header})[0] == "tankIDName") offset = 4;
                                    getpeer->protocol = std::stoi(readpipe(std::string{header})[5 + offset]);
                                    getpeer->game_version = std::stod(readpipe(std::string{header})[7 + offset]);
                                    getpeer->country = readpipe(std::string{header})[37 + offset];
                                    gt_packet(event, 0, "OnOverrideGDPRFromServer", 38, 1, 0, 1);
                                    gt_packet(event, 0, "OnSetRoleSkinsAndTitles", "000000", "000000");
                                    gt_packet(event, 0,
                                        "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
                                        hash, 
                                        "ubistatic-a.akamaihd.net", 
                                        "0098/2805202400/cache/", 
                                        "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
                                        "proto=207|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=5231383|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1"
                                    );
                                });
                                if (header.starts_with("action|refresh_item_data"sv))
                                {
                                    enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_RELIABLE));
                                    break;
                                }
                                else if (header.starts_with("action|enter_game"sv))
                                {
                                    getpeer->tankIDName.empty() ? 
                                        getpeer->visual_name = std::string{getpeer->requestedName} + "_" + std::to_string(rand(1000, 9999)) :
                                        getpeer->visual_name = std::string{getpeer->tankIDName};

                                    gt_packet(event, 0, "OnFtueButtonDataSet", 0, 0, 0, "|||||");
                                    gt_packet(event, 0, "OnEmoticonDataChanged", 0, "(wl)|─ü|0&(yes)|─é|0&(no)|─â|0&(love)|─ä|0&(oops)|─à|0&(shy)|─å|0&(wink)|─ç|0&(tongue)|─ê|0&(agree)|─ë|0&(sleep)|─è|0&(punch)|─ï|0&(music)|─î|0&(build)|─ì|0&(megaphone)|─Ä|0&(sigh)|─Å|0&(mad)|─É|0&(wow)|─æ|0&(dance)|─Æ|0&(see-no-evil)|─ô|0&(bheart)|─ö|0&(heart)|─ò|0&(grow)|─û|0&(gems)|─ù|0&(kiss)|─ÿ|0&(gtoken)|─Ö|0&(lol)|─Ü|0&(smile)|─Ç|0&(cool)|─£|0&(cry)|─¥|0&(vend)|─₧|0&(bunny)|─¢|0&(cactus)|─ƒ|0&(pine)|─ñ|0&(peace)|─ú|0&(terror)|─í|0&(troll)|─á|0&(evil)|─ó|0&(fireworks)|─ª|0&(football)|─Ñ|0&(alien)|─º|0&(party)|─¿|0&(pizza)|─⌐|0&(clap)|─¬|0&(song)|─½|0&(ghost)|─¼|0&(nuke)|─¡|0&(halo)|─«|0&(turkey)|─»|0&(gift)|─░|0&(cake)|─▒|0&(heartarrow)|─▓|0&(lucky)|─│|0&(shamrock)|─┤|0&(grin)|─╡|0&(ill)|─╢|0&(eyes)|─╖|0&(weary)|─╕|0&(moyai)|─╝|0&(plead)|─╜|0&");
                                    gt_packet(event, 0, "UpdateMainMenuTheme", 0, -68966913, -68966913);
                                    gt_packet(event, 0, "OnSetBux", getpeer->gems, 1, 0);
                                    gt_packet(event, 0, "SetHasGrowID", getpeer->tankIDName.empty() ? 0 : 1, std::string{getpeer->tankIDName}.c_str(), std::string{getpeer->tankIDName}.c_str());
                                    gt_packet(event, 0, "OnTodaysDate", 6, 5, 7351, 3);
                                    gt_packet(event, 0, "OnRequestWorldSelectMenu", "add_filter|\nadd_heading|Top Worlds<ROW2>|");
                                    gt_packet(event, 0, "OnConsoleMessage", std::format("Where would you like to go? (`w{}`` online)", peers().size()).c_str());
                                    break;
                                }
                                break;
                            }
                            case 3: 
                            {
                                if (header.starts_with("action|join_request"sv)) {
                                    std::ranges::replace(header, '\n', '|');
                                    std::string world_name = readpipe(std::string{header})[3];
                                    gt_packet(event, 0, "OnSpawn", std::format(
                                        "spawn|avatar\nnetID|{0}\nuserID|{1}\neid|\nip|\ncolrect|0|0|20|30\nposXY|1440|736\nname|{2}\ntitleIcon|\ncountry|{3}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\ntype|local",
                                        getpeer->netid, getpeer->user_id, getpeer->visual_name, std::string{getpeer->country}.c_str()));
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
	        });
    }
    return 0;
}
