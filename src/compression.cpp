#include <cmath>
#include <algorithm>

#include "compression.h"
#include "predicate.h"

double entropy(double p) {
    if (p <= 0.0 || p >= 1.0) return 0.0;
    return -p * log2(p) - (1 - p) * log2(1 - p);
}

// Evaluate if a rule matches a given data row
bool evaluate_rule(const Rule& rule, const DataRow& row) {
    for (const auto& pred : rule) {
        auto it = row.find(pred.name);
        if (it == row.end() || !it->second.has_value()) return false;
        if (pred.negated && it->second.value()) return false;
        if (!pred.negated && !it->second.value()) return false;
    }
    return true;
}

// Compute H(y | R) and P[R(x)] over dataset
std::pair<double, double> conditional_entropy(const Rule& rule, const Dataset& data) {
    int match = 0, match_old = 0;

    for (const auto& row : data) {
        if (evaluate_rule(rule, row)) {
            ++match;
            if (row.at("donor_is_old").value()) ++match_old;
        }
    }

    if (match == 0) return {0.0, 0.0};
    double p = static_cast<double>(match_old) / match;
    return {entropy(p), static_cast<double>(match) / data.size()};
}

// IG(R)
double information_gain(const Rule& rule, const Dataset& data, double base_entropy, double beta) {
    auto [h1, p] = conditional_entropy(rule, data);

    // Compute conditional entropy on negation
    int nmatch = 0, nmatch_old = 0;
    for (const auto& row : data) {
        if (!evaluate_rule(rule, row)) {
            ++nmatch;
            if (row.at("donor_is_old").value()) ++nmatch_old;
        }
    }

    double q = nmatch == 0 ? 0 : static_cast<double>(nmatch_old) / nmatch;
    double h2 = entropy(q);
    double p_neg = static_cast<double>(nmatch) / data.size();

    return base_entropy - (beta + (1 - beta) * p) * h1 - (1 - beta) * p_neg * h2;
}

std::optional<Rule> intersect_rules(const Rule& r1, const Rule& r2) {
    Rule result;
    for (const auto& p : r1) {
        auto it = r2.find(p);
        if (it != r2.end()) result.insert(p);
    }
    return result;
}

std::optional<Rule> union_rules(const Rule& r1, const Rule& r2) {
    Rule result = r1;
    for (const auto& p : r2) {
        Predicate negated = {p.name, !p.negated};
        if (result.count(negated)) return std::nullopt; // contradiction
        result.insert(p);
    }
    return result;
}

double joint_information_gain(const Rule& r1, const Rule& r2, const Dataset& data, double base_entropy, double beta) {
    // Case: R1 && R2
    Rule both = r1;
    both.insert(r2.begin(), r2.end());

    auto [h_both, p_both] = conditional_entropy(both, data);

    // Other 3 partitions
    std::vector<std::pair<Rule, double>> partitions;
    Rule r1_only = r1;
    for (const auto& p : r2) r1_only.insert({p.name, !p.negated});
    partitions.emplace_back(r1_only, 0.0);

    Rule r2_only = r2;
    for (const auto& p : r1) r2_only.insert({p.name, !p.negated});
    partitions.emplace_back(r2_only, 0.0);

    Rule neither;
    for (const auto& p : r1) neither.insert({p.name, !p.negated});
    for (const auto& p : r2) neither.insert({p.name, !p.negated});
    partitions.emplace_back(neither, 0.0);

    double sum = 0.0;
    for (auto& [rule, _] : partitions) {
        auto [h, p] = conditional_entropy(rule, data);
        sum += (1 - beta) * p * h;
    }

    return base_entropy - (beta + (1 - beta) * p_both) * h_both - sum;
}

// Main compression routine
void compress_rules(std::vector<Rule>& rules, const Dataset& data, int m, int n_rounds, double beta) {
    double base_p = 0.0;
    for (const auto& row : data) {
        if (row.at("donor_is_old").value()) {
            base_p++;
        }
    }
    base_p /= data.size();
    double base_entropy = entropy(base_p);

    for (int round=0; round<n_rounds; round++) {
        std::vector<std::tuple<int, int, double, Rule>> merges;

        for (int i = 0; i < rules.size(); ++i) {
            for (int j = i + 1; j < rules.size(); ++j) {
                double ig1 = information_gain(rules[i], data, base_entropy, beta);
                double ig2 = information_gain(rules[j], data, base_entropy, beta);
                double joint_ig = joint_information_gain(rules[i], rules[j], data, base_entropy, beta);

                merges.emplace_back(i, j, joint_ig - ig1, rules[i]);
                merges.emplace_back(i, j, joint_ig - ig2, rules[j]);

                auto intersect = intersect_rules(rules[i], rules[j]);
                auto unify = union_rules(rules[i], rules[j]);

                if (intersect) {
                    double ig_intersect = information_gain(*intersect, data, base_entropy, beta);
                    merges.emplace_back(i, j, joint_ig - ig_intersect, *intersect);
                }
                if (unify) {
                    double ig_union = information_gain(*unify, data, base_entropy, beta);
                    merges.emplace_back(i, j, joint_ig - ig_union, *unify);
                }
            }
        }

        if (merges.empty()) break;

        std::ranges::sort(merges, [](const auto& a, const auto& b) {
            return get<2>(a) < get<2>(b); // sort by delta
        });

        std::set<int> used;
        std::vector<Rule> new_rules;

        for (int i = 0; i < merges.size() && new_rules.size() < m; ++i) {
            auto [ri, rj, delta, merged] = merges[i];
            if (used.contains(ri) || used.contains(rj)) continue;
            used.insert(ri);
            used.insert(rj);
            new_rules.push_back(merged);
        }

        // Retain untouched rules
        for (int i = 0; i < rules.size(); ++i) {
            if (!used.contains(i)) new_rules.push_back(rules[i]);
        }

        rules = std::move(new_rules);
    }
}
