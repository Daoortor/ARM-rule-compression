#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <optional>
#include <cmath>
#include <algorithm>
#include <tuple>

#include "predicate.h"
#include "parser.h"


std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(line);
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::optional<bool> parse_value(const std::string& val) {
    if (val == "TRUE") return true;
    if (val == "FALSE") return false;
    return std::nullopt; // NA
}

Dataset parse_dataset(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    Dataset data;

    getline(file, line);
    std::vector<std::string> headers = split(line, '\t');

    while (getline(file, line)) {
        std::vector<std::string> values = split(line, '\t');
        DataRow row;
        for (size_t i = 0; i < headers.size(); ++i) {
            row[headers[i]] = parse_value(values[i]);
        }
        data.push_back(row);
    }

    return data;
}

Rule parse_rule_line(const std::string& line) {
    Rule rule;
    std::string lhs = line.substr(0, line.find("=>"));
    std::istringstream stream(lhs);
    std::string token;
    while (getline(stream, token, ' ')) {
        if (token == "AND" || token.empty()) continue;
        Predicate pred;
        if (token == "NOT") {
            stream >> token;
            pred = {token, true};
        } else {
            pred = {token, false};
        }
        rule.insert(pred);
    }
    return rule;
}

std::vector<Rule> parse_ruleset(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<Rule> rules;
    while (getline(file, line)) {
        if (line.empty()) continue;
        rules.push_back(parse_rule_line(line));
    }
    return rules;
}
