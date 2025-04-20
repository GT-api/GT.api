
unsigned hash{};
std::once_flag hash_init{};

void logging_in(ENetEvent event, const std::string& header)
{
    std::call_once(_peer[event.peer]->logging_in, [&]() 
    {
        std::vector<std::string> pipes = readch(header, '|');
        std::call_once(hash_init, [=]()
        {
            hash = std::ranges::fold_left(std::span(reinterpret_cast<unsigned int*>(&im_data[0]), im_data.size() / sizeof(unsigned int)), 
            0x55555555u, [](auto start, auto end){ return std::rotl(start, 5) + end; });
        });
        gt_packet(*event.peer, false, {
            "OnSuperMainStartAcceptLogonHrdxs47254722215a",
            hash,
            "ubistatic-a.akamaihd.net",
            "0098/979878696/cache/",
            "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster",
            "proto=216|choosemusic=audio/mp3/about_theme.mp3|active_holiday=19|wing_week_day=0|ubi_week_day=7|server_tick=49926747|clash_active=0|drop_lavacheck_faster=1|isPayingUser=1|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|m_clientBits=|eventButtons={\"EventButtonData\":[{\"active\":true,\"buttonAction\":\"dailychallengemenu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"itemIdIcon\":23,\"name\":\"DailyChallenge\",\"order\":1,\"rcssClass\":\"daily_challenge\",\"text\":\"\"},{\"active\":true,\"buttonAction\":\"openPiggyBank\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"PiggyBankButton\",\"order\":2,\"rcssClass\":\"piggybank\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"showdungeonsui\",\"buttonTemplate\":\"DungeonEventButton\",\"counter\":0,\"counterMax\":20,\"name\":\"ScrollsPurchaseButton\",\"order\":3,\"rcssClass\":\"scrollbank\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"show_bingo_ui\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"WinterBingoButton\",\"order\":4,\"rcssClass\":\"wf-bingo\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"show_bingo_ui\",\"buttonTemplate\":\"BaseEventButton\",\"name\":\"UbiBingoButton\",\"order\":5,\"rcssClass\":\"ubi-bingo\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"winterrallymenu\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"WinterRallyButton\",\"order\":5,\"rcssClass\":\"winter-rally\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"leaderboardBtnClicked\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"AnniversaryLeaderboardButton\",\"order\":5,\"rcssClass\":\"anniversary-leaderboard\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"euphoriaBtnClicked\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":0,\"name\":\"AnniversaryEuphoriaButton\",\"order\":5,\"rcssClass\":\"anniversary-euphoria\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"openLnySparksPopup\",\"buttonTemplate\":\"BaseEventButton\",\"counter\":0,\"counterMax\":5,\"name\":\"LnyButton\",\"order\":5,\"rcssClass\":\"cny\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"ShowValentinesQuestDialog\",\"buttonTemplate\":\"EventButtonWithCounter\",\"counter\":0,\"counterMax\":100,\"name\":\"ValentinesButton\",\"order\":5,\"rcssClass\":\"valentines_day\",\"text\":\"\"},{\"active\":false,\"buttonAction\":\"openStPatrickPiggyBank\",\"buttonTemplate\":\"BaseEventButton\",\"name\":\"StPatrickPBButton\",\"order\":5,\"rcssClass\":\"st_patrick_event\",\"text\":\"\"}]}"
        });
        if (pipes[2] == "ltoken")
        {
            std::string decoded = base64Decode(pipes[3]);
            if (auto pos = decoded.find("growId="); pos not_eq std::string::npos) 
            {
                pos += 7; // @note size of "growId="
                std::size_t ampersand = decoded.find('&', pos);
                _peer[event.peer]->ltoken[0] = strdup(decoded.substr(pos, ampersand - pos).c_str());
            }

            if (auto pos = decoded.find("password="); pos not_eq std::string::npos) 
            {
                pos += 9; // @note size of "password="
                _peer[event.peer]->ltoken[1] = strdup(decoded.substr(pos).c_str());
            }
        }
    });
}