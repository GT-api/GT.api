
void find(ENetEvent& event, const std::string& text)
{
    std::string result{};
    std::string item = "dirt";
    short i = 0;
    for (const auto& im : items) 
    {
        if (im.first % 2 not_eq 0) continue; // @todo add filter to include/exclude seeds.
        std::string small_name = im.second.raw_name;
        std::ranges::transform(small_name, small_name.begin(), [](char c) { return std::tolower(c); });
        if (small_name.contains(item)) 
        {
            if (i >= 8) result += "\nadd_button_with_icon||END_LIST|noflags|0||", i = 0;
            result += std::format("\nadd_button_with_icon|find_{0}||staticBlueFrame,no_padding_x,enabled|{0}||", im.first).c_str();
            ++i;
        }
    }
    result += "\nadd_button_with_icon||END_ROW|noflags|0||";

    gt_packet(*event.peer, false, "OnDialogRequest", std::format(R"(set_default_color|`o{}
add_spacer|small|
end_dialog|find_||Close|)", result).c_str());
}