#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "predicate.h"

double entropy(double p);

bool evaluate_rule(const Rule& rule, const DataRow& row);

std::pair<double, double> conditional_entropy(const Rule& rule, const Dataset& data, bool negate=false);

double information_gain(const Rule& rule, const Dataset& data, double base_entropy, double beta);

std::optional<Rule> intersect_rules(const Rule& r1, const Rule& r2);

std::optional<Rule> union_rules(const Rule& r1, const Rule& r2);

double joint_information_gain(const Rule& r1, const Rule& r2, const Dataset& data, double base_entropy, double beta);

void compress_rules(Ruleset& rules, const Dataset& data, int m, int n_rounds, double beta);

#endif //COMPRESSION_H
