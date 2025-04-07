#ifndef PREDICATE_H
#define PREDICATE_H

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>


struct Predicate {
    std::string name;
    bool negated{};

    bool operator==(const Predicate& other) const;

    bool operator<(const Predicate& other) const;
};

struct PredicateHash {
    size_t operator()(const Predicate& p) const;
};

using Rule = std::set<Predicate>;
using DataRow = std::unordered_map<std::string, std::optional<bool>>;
using Dataset = std::vector<DataRow>;

#endif //PREDICATE_H
