
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
