#ifndef HUNTERS_API_LP_PARSER_HPP
#define HUNTERS_API_LP_PARSER_HPP

#include "model/Model.hpp"
#include <string>

namespace hunters {

class LPParser {
public:
    // Parse standard .lp format file
    static Model parse_file(const std::string& filepath);

    // Parse from string buffer
    static Model parse_string(const std::string& content);
};

} // namespace hunters

#endif // HUNTERS_API_LP_PARSER_HPP
