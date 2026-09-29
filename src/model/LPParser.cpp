#include "api/LPParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace hunters {

namespace {

std::string to_upper(const std::string& s) {
    std::string res = s;
    for (char& c : res) c = std::toupper(static_cast<unsigned char>(c));
    return res;
}

// Tokenizes a line into tokens, stripping comments starting with '\'
std::vector<std::string> tokenize_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::string clean = line;
    size_t comment_pos = clean.find('\\');
    if (comment_pos != std::string::npos) {
        clean = clean.substr(0, comment_pos);
    }
    std::istringstream iss(clean);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Parses a linear expression into (var_name, coefficient) pairs
void parse_linear_expr(const std::vector<std::string>& tokens,
                       size_t start_idx,
                       size_t end_idx,
                       std::vector<std::pair<std::string, double>>& terms) {
    double current_sign = 1.0;
    double current_coef = 1.0;
    bool has_coef = false;

    for (size_t i = start_idx; i < end_idx; ++i) {
        const std::string& tok = tokens[i];
        if (tok == "+") {
            current_sign = 1.0;
        } else if (tok == "-") {
            current_sign = -1.0;
        } else {
            // Check if token starts with + or -
            std::string sub = tok;
            double local_sign = current_sign;
            if (sub[0] == '+') {
                local_sign = 1.0;
                sub = sub.substr(1);
            } else if (sub[0] == '-') {
                local_sign = -1.0;
                sub = sub.substr(1);
            }

            // Check if purely numeric
            char* end_ptr = nullptr;
            double val = std::strtod(sub.c_str(), &end_ptr);
            if (end_ptr == sub.c_str() + sub.size()) {
                // It's a number
                current_coef = local_sign * val;
                has_coef = true;
                current_sign = 1.0;
            } else {
                // It's a variable or coef+var (e.g. 5x)
                if (end_ptr != sub.c_str() && end_ptr != nullptr) {
                    double coef_part = local_sign * val;
                    std::string var_part = std::string(end_ptr);
                    terms.emplace_back(var_part, coef_part);
                    current_coef = 1.0;
                    has_coef = false;
                    current_sign = 1.0;
                } else {
                    double final_coef = has_coef ? current_coef : local_sign;
                    terms.emplace_back(sub, final_coef);
                    current_coef = 1.0;
                    has_coef = false;
                    current_sign = 1.0;
                }
            }
        }
    }
}

} // anonymous namespace

Model LPParser::parse_string(const std::string& content) {
    Model model("ParsedLP");
    std::istringstream stream(content);
    std::string line;

    enum class Section {
        NONE,
        OBJECTIVE,
        CONSTRAINTS,
        BOUNDS,
        INTEGERS,
        BINARIES,
        END
    };

    Section current_section = Section::NONE;
    std::vector<std::pair<std::string, double>> obj_terms;

    auto get_or_add_var = [&](const std::string& vname) -> int {
        auto it = model.var_name_to_idx.find(vname);
        if (it != model.var_name_to_idx.end()) return it->second;
        return model.add_variable(vname, 0.0, Variable::INF, 0.0, VarType::CONTINUOUS);
    };

    while (std::getline(stream, line)) {
        std::vector<std::string> tokens = tokenize_line(line);
        if (tokens.empty()) continue;

        std::string first_upper = to_upper(tokens[0]);

        if (first_upper == "MINIMIZE" || first_upper == "MIN") {
            model.minimize();
            current_section = Section::OBJECTIVE;
            if (tokens.size() > 1) {
                parse_linear_expr(tokens, 1, tokens.size(), obj_terms);
            }
            continue;
        } else if (first_upper == "MAXIMIZE" || first_upper == "MAX") {
            model.maximize();
            current_section = Section::OBJECTIVE;
            if (tokens.size() > 1) {
                parse_linear_expr(tokens, 1, tokens.size(), obj_terms);
            }
            continue;
        } else if (first_upper == "SUBJECT" || first_upper == "SUCH" || first_upper == "ST" || first_upper == "S.T.") {
            current_section = Section::CONSTRAINTS;
            continue;
        } else if (first_upper == "BOUNDS" || first_upper == "BOUND") {
            current_section = Section::BOUNDS;
            continue;
        } else if (first_upper == "GENERALS" || first_upper == "GENERAL" || first_upper == "INTEGERS" || first_upper == "INT" || first_upper == "GEN") {
            current_section = Section::INTEGERS;
            for (size_t i = 1; i < tokens.size(); ++i) {
                int v_idx = get_or_add_var(tokens[i]);
                model.variables[v_idx].type = VarType::INTEGER;
            }
            continue;
        } else if (first_upper == "BINARIES" || first_upper == "BINARY" || first_upper == "BIN") {
            current_section = Section::BINARIES;
            for (size_t i = 1; i < tokens.size(); ++i) {
                int v_idx = get_or_add_var(tokens[i]);
                model.variables[v_idx].type = VarType::BINARY;
                model.variables[v_idx].lower_bound = 0.0;
                model.variables[v_idx].upper_bound = 1.0;
            }
            continue;
        } else if (first_upper == "END") {
            current_section = Section::END;
            break;
        }

        // Process line based on active section
        if (current_section == Section::OBJECTIVE) {
            size_t start_idx = 0;
            if (tokens[0].back() == ':') {
                start_idx = 1;
            } else if (tokens.size() > 1 && tokens[1] == ":") {
                start_idx = 2;
            }
            if (start_idx < tokens.size()) {
                parse_linear_expr(tokens, start_idx, tokens.size(), obj_terms);
            }
        } else if (current_section == Section::CONSTRAINTS) {
            // Find constraint name if colon present
            std::string c_name = "";
            size_t start_idx = 0;
            if (tokens[0].back() == ':') {
                c_name = tokens[0].substr(0, tokens[0].size() - 1);
                start_idx = 1;
            } else if (tokens.size() > 1 && tokens[1] == ":") {
                c_name = tokens[0];
                start_idx = 2;
            }

            // Find relational operator
            size_t op_idx = tokens.size();
            ConstraintSense sense = ConstraintSense::LESS_EQUAL;
            for (size_t i = start_idx; i < tokens.size(); ++i) {
                if (tokens[i] == "<=" || tokens[i] == "<") {
                    sense = ConstraintSense::LESS_EQUAL;
                    op_idx = i;
                    break;
                } else if (tokens[i] == ">=" || tokens[i] == ">") {
                    sense = ConstraintSense::GREATER_EQUAL;
                    op_idx = i;
                    break;
                } else if (tokens[i] == "=" || tokens[i] == "==") {
                    sense = ConstraintSense::EQUAL;
                    op_idx = i;
                    break;
                }
            }

            if (op_idx < tokens.size()) {
                std::vector<std::pair<std::string, double>> terms;
                parse_linear_expr(tokens, start_idx, op_idx, terms);
                double rhs = (op_idx + 1 < tokens.size()) ? std::stod(tokens[op_idx + 1]) : 0.0;

                std::vector<int> v_indices;
                std::vector<double> coefs;
                for (const auto& term : terms) {
                    int v_idx = get_or_add_var(term.first);
                    v_indices.push_back(v_idx);
                    coefs.push_back(term.second);
                }
                model.add_constraint(c_name, v_indices, coefs, sense, rhs);
            }
        } else if (current_section == Section::BOUNDS) {
            // Handles bounds like: 0 <= x1 <= 100, x1 >= 5, x1 <= 50, x1 free
            if (tokens.size() >= 3 && to_upper(tokens[1]) == "FREE") {
                int v_idx = get_or_add_var(tokens[0]);
                model.variables[v_idx].lower_bound = -Variable::INF;
                model.variables[v_idx].upper_bound = Variable::INF;
            } else if (tokens.size() == 5 && (tokens[1] == "<=" || tokens[1] == "<") && (tokens[3] == "<=" || tokens[3] == "<")) {
                double lb = std::stod(tokens[0]);
                int v_idx = get_or_add_var(tokens[2]);
                double ub = std::stod(tokens[4]);
                model.variables[v_idx].lower_bound = lb;
                model.variables[v_idx].upper_bound = ub;
            } else if (tokens.size() >= 3) {
                if (tokens[1] == "<=" || tokens[1] == "<") {
                    int v_idx = get_or_add_var(tokens[0]);
                    double ub = std::stod(tokens[2]);
                    model.variables[v_idx].upper_bound = ub;
                } else if (tokens[1] == ">=" || tokens[1] == ">") {
                    int v_idx = get_or_add_var(tokens[0]);
                    double lb = std::stod(tokens[2]);
                    model.variables[v_idx].lower_bound = lb;
                } else if (tokens[1] == "=" || tokens[1] == "==") {
                    int v_idx = get_or_add_var(tokens[0]);
                    double val = std::stod(tokens[2]);
                    model.variables[v_idx].lower_bound = val;
                    model.variables[v_idx].upper_bound = val;
                }
            }
        } else if (current_section == Section::INTEGERS) {
            for (const auto& tok : tokens) {
                int v_idx = get_or_add_var(tok);
                model.variables[v_idx].type = VarType::INTEGER;
            }
        } else if (current_section == Section::BINARIES) {
            for (const auto& tok : tokens) {
                int v_idx = get_or_add_var(tok);
                model.variables[v_idx].type = VarType::BINARY;
                model.variables[v_idx].lower_bound = 0.0;
                model.variables[v_idx].upper_bound = 1.0;
            }
        }
    }

    // Apply parsed objective terms
    for (const auto& term : obj_terms) {
        int v_idx = get_or_add_var(term.first);
        model.variables[v_idx].obj_coefficient = term.second;
    }

    return model;
}

Model LPParser::parse_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open LP file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_string(buffer.str());
}

} // namespace hunters
