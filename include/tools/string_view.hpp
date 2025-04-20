
/*
 @param str the whole string
 @param c the char to search for and split
 @return std::vector<std::string> storing each split off from c found in the str.
*/
std::vector<std::string> readch(const std::string& str, const char& c) 
{
    std::vector<std::string> separations;
    separations.reserve(std::count(str.begin(), str.end(), c) + 1); 
    for (auto&& part : str bitor std::views::split(c))
        separations.emplace_back(std::string(std::ranges::begin(part), std::ranges::end(part)));
    return separations;
}

/* @return true if string contains ONLY alpha [a, b, c] or digits [1, 2, 3] */
bool alpha(const std::string& str) 
{
    for (char c : str)
        if (not std::isalnum(static_cast<unsigned char>(c)))
            return false;
    return true;
}

constexpr std::array<int, 256> createLookupTable() {
    std::array<int, 256> table{};
    table.fill(-1);
    constexpr std::string_view base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (size_t i = 0; i < 64; ++i) {
        table[static_cast<unsigned char>(base64_chars[i])] = static_cast<int>(i);
    }
    return table;
}

std::string base64Decode(std::string_view encoded) {
    constexpr auto lookupTable = createLookupTable();
    std::string decoded;
    decoded.reserve(encoded.size() * 3 / 4);
    
    int val = 0, valb = -8;
    for (char c : encoded) {
        if (lookupTable[c] == -1) continue;
        val = (val << 6) + lookupTable[c];
        valb += 6;
        if (valb >= 0) {
            decoded.push_back(static_cast<char>((val >> valb) & 0xff));
            valb -= 8;
        }
    }
    return decoded;
}