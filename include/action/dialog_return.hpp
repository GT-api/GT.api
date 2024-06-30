
void dialog_return(ENetEvent& event, const std::string header) 
{
    std::vector<std::string> pipes = readpipe(header);
    std::string dialog_name = pipes[3];
    if (pipes.size() > 3)
        pipes.erase(pipes.begin(), pipes.begin() + 4);
    else return; // button has no name.
    if (dialog_name == "growid_apply" and pipes[0] == "logon" and pipes[2] == "password" and pipes[4] == "password_verify" and pipes[6] == "email")
    {
        std::string& logon = pipes[1];
        std::string& password = pipes[3];
        std::string& password_verify = pipes[5];
        if (password not_eq password_verify) growid(event, header, "`4Oops!``  Passwords don't match.  Try again.");
        else if (password.size() < 8 or password.size() > 18) growid(event, header, "`4Oops!``  Your password must be between `$8`` and `$18`` characters long.");
        else if (logon.size() < 3 or logon.size() > 18) growid(event, header, "`4Oops!``  Your `wGrowID`` must be between `$3`` and `$18`` characters long.");
        else if (read_peer(event, logon) == true or alpha(logon)) growid(event, header, std::format("`4Oops!`` The name `w{}`` is unavailable.  Please choose a different name.", logon));
        else 
        {
            getpeer->tankIDName = logon;
            getpeer->tankIDPass = password;
            getpeer->nickname = (getpeer->tankIDName.empty()) ? getpeer->requestedName : getpeer->tankIDName;
            gt_packet(*event.peer, 0, false, "SetHasGrowID", 1, getpeer->tankIDName.c_str(), getpeer->tankIDPass.c_str());
            gt_packet(*event.peer, 0, false, "OnNameChange", getpeer->nickname.c_str());
            register_peer(event);
        }
    }
}