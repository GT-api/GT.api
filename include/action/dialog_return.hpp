
void dialog_return(ENetEvent event, const std::string& header) 
{
    std::vector<std::string> pipes = readch(header, '|');
    std::string dialog_name = pipes[3];
    if (pipes.size() > 3)
        pipes.erase(pipes.begin(), pipes.begin() + 4);
    else return; // if button has no name.
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
        const short id = stoi(pipes[1]); // @note comfirm they have the item without extra iteration.
        const short count = stoi(pipes[4]);
        _peer[event.peer]->emplace(slot{id, static_cast<short>(count * -1)}); // @note take away
        inventory_visuals(*event.peer);
        float x_nabor = (_peer[event.peer]->facing_left ? _peer[event.peer]->pos[0] - 1 : _peer[event.peer]->pos[0] + 1); // @note get the tile next to peer. so like O|
        drop_visuals(event, {id, count}, {x_nabor, _peer[event.peer]->pos[1]});
    }
    else if (dialog_name == "find" and pipes[0] == "buttonClicked" and pipes[1].starts_with("searchableItemListButton"))
    {
        _peer[event.peer]->emplace({static_cast<short>(stoi(readch(pipes[1], '_')[1])), 200});
        inventory_visuals(*event.peer);
    }
}