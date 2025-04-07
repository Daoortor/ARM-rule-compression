#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include "predicate.h"
#include "parser.h"
#include "compression.h"


int main() {
    Dataset data = parse_dataset("../data/dataset.tsv");
    std::vector<Rule> rules = parse_ruleset("../data/rules.txt");

    double beta = 0.5;
    int m = 10; // number of merges per round
    int n_rounds = 2;
    compress_rules(rules, data, m, n_rounds, beta);

    // Output final compressed rules
    for (const auto& rule : rules) {
        for (auto it = rule.begin(); it != rule.end(); ++it) {
            if (it != rule.begin()) {
                std::cout << " AND ";
            }
            if (it->negated) {
                std::cout << "NOT ";
            }
            std::cout << it->name;
        }
        std::cout << " => donor_is_old" << std::endl;
    }

    return 0;
}
