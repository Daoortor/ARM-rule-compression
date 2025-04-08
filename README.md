# ARM rule compression
## Task
You are provided with a portion of a real dataset describing donors categorized into two age groups: young and old. The dataset includes various biomarkers measured from donor blood. Each biomarker is represented as a logical predicate that evaluates to true when the biomarker's value is relatively high.

A system has generated a set of rules to identify which biomarkers (or biomarker combinations) are indicative of an "old" donor. Your objective is to compress this rule set while preserving the key insights.

Each rule follows the structure ```LHS => donor_is_old```. The left-hand side (LHS) contains one or more predicates, which may appear in their normal or negated form, combined using the AND keyword. Negations are applied only to individual predicates, using the keyword NOT, which must immediately precede the predicate name. The right-hand side (RHS) always consists solely of the predicate ```donor_is_old```.

## Solution
### Notation
Data points can be viewed as tuples $(X_i, y_i)$.
$y_i$ is $1$ if ```donor_is_old```, and $0$ otherwise (we assume that ```donor_is_old``` is never $\text{NA}$).
$X_i = (x_{i1}, x_{i2}, ..., x_{id})$ are the values of other features; $x_i \in \{0, 1, \text{NA}\}$. The dataset can be viewed as $(X, Y)$, where $X = (x_1, x_2, ..., x_n)$, $Y = (y_1, y_2, ..., y_n)$, and $(x_i, y_i)$ is the $i$th data point. <br>
Writing a rule as ```R => donor_is_old```, ```R``` can be viewed as a random event  
$\{x_i \in X\colon R(x_i)\}$ on the sample set $X$, with the convention that  
$R(x)$ doesn't hold whenever any of the features present in $R$ are $\text{NA}$  
in $x$. In the same fashion, the predicate ```donor_is_old``` can be viewed as a random   
event $y$.
### Scoring mechanism for rules
Now we can measure the "interestingness" of a rule $R$ by the amount of  
information the value of $R(x)$ gives about the value of $y_i$. More formally, we consider the metric
$$IG(R) = H(y) - (\beta + (1 - \beta) P[R(x)]) \cdot H(y|R(x)) - (1 - \beta)P[\neg R(x)] \cdot H(y|\neg R(x))$$
, where $H$ is the entropy and $\beta > 0$ is a hyperparameter. This is similar to information gain used in decision trees.
- If $R$ is independent from $y$, $IG(R) = H(y)(1 - P[R(x)] - P[\neg R(x)]) = 0$.
- If $R \implies y$, $IG(R) = H(y) - P[R(x)] \cdot 0 - P[\neg R(x)] \cdot 0 = H(y)$.
- For $\beta = 1$, our measure only depends on $P[y|R(x)]$, and thus doesn't penalize overly specific rules (i. e. when $P[R(x)]$ is small). For $\beta = 0$, our measure is the information gain of the rule $R \iff y$. Different values of $\beta$ provide a tradeoff between these extremes.
- $IG(R) = IG(\neg R)$. <br>
  In a similar fashion, we can calculate joint information gain of two rules:
  $$
  IG(R_1, R_2) = H(y) -  (\beta + (1 - \beta) P[R_1(x) \land R_2(x)]) \cdot H(y|R_1(x) \land R_2(x)) - \sum_{R \in S} (1 - \beta) P[R] \cdot H[y|R]
  $$
  , where $$S = \{R_1(x) \land \neg R_2(x), \neg R_1(x) \land R_2(x), \neg R_1(x) \land \neg R_2(x)\}$$
  If $R_1 = R_2 = R$, both middle terms become zero, and two other terms become the corresponding terms in $(1)$, therefore, $IG(R, R) = IG(R)$. <br>
  We could continue and calculate $IG(R_1, R_2, ..., R_k)$ for arbitrary $k$. However, at some point the set of rules will overfit the dataset, and, knowing the values of $R_1(x), ..., R_k(x)$, we will be able to predict $y(x)$ exactly: $IG(R_1, R_2, ..., R_k) = H(y)$. In this case, $IG$ is insensitive to the choice of rules, making it unsuitable for ruleset compression.
### Ruleset compression
Consider two rules $R_1$, $R_2$. Let's calculate $IG(R_1)$, $IG(R_2)$, $IG(R_1 \cdot R_2)$, $IG(R_1 + R_2)$. Here, $R_1 \cdot R_2$ is the rule obtained by taking the union of terms from $R_1$ and $R_2$ and removing contradictory terms, and $R_1 + R_2$ is the rule obtained by taking the intersection of terms from $R_1$ and $R_2$. If one of these is close enough to $IG(R_1, R_2)$, this means we can replace $R_1$ and $R_2$ with the corresponding "merged" rule $R_{1,2}$ with minimal loss of information. <br>
We proceed in rounds. During each round:
- calculate $\Delta_{i, j} = IG(R_i, R_j) - IG(R_{i, j})$ of each $R_i, R_j$ in the current ruleset;
- choose $m$ pairs with the lowest $\Delta_{i, j}$ and merge them one-by-one. <br>
The algorithm is stopped once the ruleset is compressed enough. Low values of $m$ result in higher accuracy, but also higher computational costs.

### Further heuristics
Possible future heuristics:

- Compressing rules $R_1, R_2$ s. t. $R_1(x) => R_2(x) \forall x$
- More complex rule merging. For example, if $R_1$ is more significant than $R_2$, the merged rule might need to be closer to $R_1$ than to $R_2$
