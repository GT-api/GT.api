
void drop(ENetEvent event, const std::string& header)
{
    std::string id{readch(std::string{header}, '|')[4]};
    for (const auto& slot : _peer[event.peer]->slots)
        if (slot.id == stoi(id)) {
            gt_packet(*event.peer, false, "OnDialogRequest", 
std::format(R"(set_default_color|`o
add_label_with_icon|big|`wDrop {0}``|left|{1}|
add_textbox|How many to drop?|left|
add_text_input|count||{2}|5|
embed_data|itemID|{1}
end_dialog|drop_item|Cancel|OK|)", items[stoull(id)].raw_name, id, slot.count).c_str());
            break; // early exit
        }
}