
void logging_in(ENetEvent& event, const std::string& header)
{
    std::call_once(getpeer->logging_in, [&]() 
    {
        std::vector<std::string> pipes = readpipe(header);
        gt_packet(*event.peer, 0, true,
            "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
            hash, 
            "ubistatic-a.akamaihd.net",
            "0098/12619900/cache/", 
            "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
            "proto=209|choosemusic=audio/mp3/about_theme.mp3|active_holiday=6|wing_week_day=0|ubi_week_day=0|server_tick=28677968|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|"
        );
    });
}