
void find(ENetEvent& event, const std::string& text)
{
    /* @note use less memory than housing it as a string and/or formatted string (std::format)- I know it is ugly but this is the best choice. */
    gt_packet(*event.peer, false, "OnDialogRequest", "set_default_color|`o\nadd_text_input|name|Search: ||26|\nadd_spacer|small|\nend_dialog|find|Close|Comfirm|");
}