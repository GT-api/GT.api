
class block {
    public:
    unsigned short fg{items[0].id}, bg{items[0].id};
    unsigned flags{0x00000000};

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
