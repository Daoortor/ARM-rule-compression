#ifndef PARSER_H
#define PARSER_H

std::vector<std::string> split(const std::string& line, char delimiter);

std::optional<bool> parse_value(const std::string& val);

Dataset parse_dataset(const std::string& filename);

Rule parse_rule_line(const std::string& line);

Ruleset parse_ruleset(const std::string& filename);

#endif //PARSER_H
