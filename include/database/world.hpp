
class block {
    public:
    short fg{0}, bg{0};
    unsigned flags{0};

};

class floating {
    public:
    int id = 0, count = 0, uid = 0, x = 0, y = 0;
};

class world {
    public:
    short x{100}, y{60};
    std::string name{};
    short visitors{0};
    std::vector<floating> floating;
    std::vector<block> blocks;
};

void OnRequestWorldSelectMenu(ENetEvent event) {
    auto section = [](auto& range, const char* color) 
    {
        std::string result;
        for (const auto& name : range)
            if (not name.empty()) /* some may be stored empty but still an object. e.g. std::array */
                result += std::format("add_floater|{0}|0|0.5|{1}\n", name, color);
        if (not result.empty())
            result.pop_back(); 
        return result;
    };
    gt_packet(*event.peer, 0, "OnRequestWorldSelectMenu", std::format(
        "add_filter|\nadd_heading|Top Worlds<ROW2>|{0}\nadd_heading|My Worlds<CR>|{1}\nadd_heading|Recently Visited Worlds<CR>|{2}",
        "", section(getpeer->locked_worlds, "2147418367"), section(getpeer->recent_worlds, "3417414143")).c_str());
    gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("Where would you like to go? (`w{}`` online)", peers().size()).c_str());
}