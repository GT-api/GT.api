
void dialog_return(ENetEvent event, const std::string& header) 
{
    std::vector<std::string> pipes = readpipe(header);
    std::string dialog_name = pipes[3];
    if (pipes.size() > 3)
        pipes.erase(pipes.begin(), pipes.begin() + 4);
    else return; // button has no name.
    if (dialog_name == "growid_apply" and pipes[0] == "logon" and pipes[2] == "password" and pipes[4] == "password_verify" and pipes[6] == "email")
    {
        const std::string& logon = pipes[1];
        const std::string& password = pipes[3];
        const std::string& password_verify = pipes[5];
        if (password not_eq password_verify) growid(event, header, "`4Oops!``  Passwords don't match.  Try again.");
        else if (password.size() < 8 or password.size() > 18) growid(event, header, "`4Oops!``  Your password must be between `$8`` and `$18`` characters long.");
        else if (logon.size() < 3 or logon.size() > 18) growid(event, header, "`4Oops!``  Your `wGrowID`` must be between `$3`` and `$18`` characters long.");
        else if (alpha(logon)) growid(event, header, std::format("`4Oops!`` The name `w{}`` is unavailable.  Please choose a different name.", logon));
    }
    else if (dialog_name == "drop_item" and pipes[0] == "itemID" and pipes[3] == "count")
    {
        const short id = stoi(pipes[1]), count = stoi(pipes[4]);
        auto slot = std::find_if(getpeer->slots.begin(), getpeer->slots.end(), 
            [&](const auto& slot) { return slot.id == id and slot.count >= count; });
        if (slot not_eq getpeer->slots.end()) 
        {
            slot->count -= count;
            drop_visuals(event, id, count);
            inventory_visuals(*event.peer);
        }
    }
    else if (dialog_name == "find" and pipes[0] == "name")
    {
        const std::string& name = pipes[1];
        std::string result{};
        short i = 0;
        result += "\nadd_text_input|name|Search: ||26|"; // @todo find am appropriate max length
        for (const auto& im : items) 
        {
            if (im.first % 2 not_eq 0) continue; // @todo add filter to include/exclude seeds.
            std::string small_name = im.second.raw_name;
            std::ranges::transform(small_name, small_name.begin(), [](char c) { return std::tolower(c); });
            if (small_name.contains(name)) 
            {
                if (i >= 8) result += "\nadd_button_with_icon||END_ROW|noflags|0||", i = 0; // @todo 8 is the max in growtopia dialogs.
                result += std::format("\nadd_button_with_icon|find_{0}||staticBlueFrame,no_padding_x,enabled|{0}||", im.first).c_str();
                ++i;
            }
        }
        result += "\nadd_button_with_icon||END_ROW|noflags|0||"; // @note if it ends up less then 8 it will be on the same row as Cancel & Comfirm...

        gt_packet(*event.peer, false, "OnDialogRequest", std::format(R"(set_default_color|`o{}
add_spacer|small|
end_dialog|find|Cancel|Comfirm|)", result).c_str());
    }
}