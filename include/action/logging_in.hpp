
void logging_in(ENetEvent& event, const std::string& header)
{
    std::call_once(getpeer->logging_in, [&]() 
    {
        std::vector<std::string> pipes = readpipe(header);
        if (pipes[0] == "tankIDName")
        if (read_peer(event, pipes[1]) == false or pipes[3] not_eq getpeer->tankIDPass)
        {
            packet(*event.peer, "action|set_url\nurl|https://www.growtopiagame.com/account.php\nlabel|`$Retrieve lost password``");
            packet(*event.peer, "action|log\nmsg|`4Unable to log on:`` That `wGrowID`` doesn't seem valid, or the password is wrong.  If you don't have one, press `wCancel``, un-check `w'I have a GrowID'``, then click `wConnect``.");
            packet(*event.peer, "action|logon_fail\n");
            return;
        }
        getpeer->requestedName = pipes[1] + "_" + std::to_string(::seed().fast(100, 999));
        short offset = getpeer->tankIDName.empty() ? 0 : 4;
        if (pipes[7 + offset] not_eq "4.59")
        {
            packet(*event.peer, "action|log\nmsg|`4UPDATE REQUIRED!`` : The `$V4.59`` update is now available for your device.  Go get it!  You'll need to install it before you can play online.");
            packet(*event.peer, "action|set_url\nurl|https://growtopiagame.com/Growtopia-Installer.exe\nlabel|Download Latest Version");
            packet(*event.peer, "action|logon_fail\n");
            return;
        }
        getpeer->country = pipes[37 + offset];
        getpeer->user_id = std::stoi(pipes[43 + offset]);
        gt_packet(*event.peer, 0, true,
            "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
            hash, 
            "ubistatic-a.akamaihd.net",
            "0098/2521452/cache/", 
            "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
            "proto=208|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=29681641|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|"
        );
    });
}