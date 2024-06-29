
void logging_in(ENetEvent& event, std::string header)
{
    std::call_once(getpeer->logging_in, [&]() 
    {
        std::vector<std::string> read_once = readpipe(header); 
        seed random{};
        read_once[0] == "requestedName" ? 
            getpeer->requestedName = read_once[1] + "_" + std::to_string(random.fast(100, 999)) :
            getpeer->tankIDName = read_once[1];
        if (not getpeer->tankIDName.empty() and getpeer->tankIDPass not_eq read_once[3])
        {
            gt_packet(*event.peer, 0, false, "OnConsoleMessage", "`4Unable to log on:`` That `wGrowID`` doesn't seem valid, or the password is wrong.  If you don't have one, press `wCancel``, un-check `w'I have a GrowID'``, then click `wConnect``.");
            enet_peer_disconnect_later(event.peer, 0);
            return;
        }
        short offset = getpeer->tankIDName.empty() ? 0 : 4;
        getpeer->country = read_once[37 + offset];
        gt_packet(*event.peer, 0, true,
            "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
            hash, 
            "ubistatic-a.akamaihd.net", 
            "0098/2521452/cache/", 
            "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
            "proto=208|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=8643774|clash_active=1|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|"
        );
    });
}