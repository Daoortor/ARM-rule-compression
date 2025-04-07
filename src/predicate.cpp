#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "predicate.h"


bool Predicate::operator==(const Predicate &other) const {
    return name == other.name && negated == other.negated;
}

bool Predicate::operator<(const Predicate &other) const {
    return tie(name, negated) < tie(other.name, other.negated);
}
