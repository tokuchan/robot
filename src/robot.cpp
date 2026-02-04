#include <boost/algorithm/string.hpp>
#include <string>

std::string to_upper_string(const std::string& text) {
    std::string result = text;
    boost::to_upper(result);
    return result;
}

std::string to_lower_string(const std::string& text) {
    std::string result = text;
    boost::to_lower(result);
    return result;
}