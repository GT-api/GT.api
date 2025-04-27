#include "database/peer.hpp"
#include "database/world.hpp"
#include "action/dialog_return.hpp"

#include "tools/string_view.hpp"

void dialog_return(ENetEvent event, const std::string& header) 
{
    std::vector<std::string> pipes = readch(header, '|');
    std::string dialog_name = pipes[3];
    if (pipes.size() > 3)
        pipes.erase(pipes.begin(), pipes.begin() + 4);
    else return; // if button has no name.
    if (dialog_name == "drop_item" and pipes[0] == "itemID" and pipes[3] == "count")
    {
        const short id = stoi(pipes[1]); // @note comfirm they have the item without extra iteration.
        const short count = stoi(pipes[4]);
        _peer[event.peer]->emplace(slot{id, static_cast<short>(count * -1)}); // @note take away
        inventory_visuals(event);
        float x_nabor = (_peer[event.peer]->facing_left ? 
            _peer[event.peer]->pos[0] - 1 : 
            _peer[event.peer]->pos[0] + 1); // @note get the tile next to peer. so like O|
        drop_visuals(event, {id, count}, {x_nabor, _peer[event.peer]->pos[1]});
    }
    else if (dialog_name == "find" and pipes[0] == "buttonClicked" and pipes[1].starts_with("searchableItemListButton"))
    {
        _peer[event.peer]->emplace(slot{static_cast<short>(stoi(readch(pipes[1], '_')[1])), 200});
        inventory_visuals(event);
    }
    else if (dialog_name == "door_edit" and pipes[6] == "door_name" and pipes[8] == "door_target" and pipes[10] == "door_id")
    {
        worlds[_peer[event.peer]->recent_worlds.back()].blocks[stoi(pipes[4])/* tiley */ * 100 + stoi(pipes[1])/* tilex */].label = pipes[7];
    }
    else if (dialog_name == "sign_edit" and pipes[6] == "sign_text")
    {
        worlds[_peer[event.peer]->recent_worlds.back()].blocks[stoi(pipes[4])/* tiley */ * 100 + stoi(pipes[1])/* tilex */].label = pipes[7];
    }
}