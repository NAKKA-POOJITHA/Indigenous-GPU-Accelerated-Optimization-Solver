\ Power Grid Unit Commitment & Economic Dispatch MILP
\ Minimize no-load startup/running cost + incremental fuel cost across 3 time periods
Minimize
 obj: 500 u1_t1 + 800 u2_t1 + 300 u3_t1 + 500 u1_t2 + 800 u2_t2 + 300 u3_t2 + 500 u1_t3 + 800 u2_t3 + 300 u3_t3 + 25 p1_t1 + 18 p2_t1 + 35 p3_t1 + 25 p1_t2 + 18 p2_t2 + 35 p3_t2 + 25 p1_t3 + 18 p2_t3 + 35 p3_t3

Subject To
 load_balance_t1: p1_t1 + p2_t1 + p3_t1 >= 450
 load_balance_t2: p1_t2 + p2_t2 + p3_t2 >= 700
 load_balance_t3: p1_t3 + p2_t3 + p3_t3 >= 550

 min_gen_1_t1: p1_t1 - 50 u1_t1 >= 0
 max_gen_1_t1: p1_t1 - 250 u1_t1 <= 0
 min_gen_2_t1: p2_t1 - 100 u2_t1 >= 0
 max_gen_2_t1: p2_t1 - 500 u2_t1 <= 0
 min_gen_3_t1: p3_t1 - 20 u3_t1 >= 0
 max_gen_3_t1: p3_t1 - 150 u3_t1 <= 0

 min_gen_1_t2: p1_t2 - 50 u1_t2 >= 0
 max_gen_1_t2: p1_t2 - 250 u1_t2 <= 0
 min_gen_2_t2: p2_t2 - 100 u2_t2 >= 0
 max_gen_2_t2: p2_t2 - 500 u2_t2 <= 0
 min_gen_3_t2: p3_t2 - 20 u3_t2 >= 0
 max_gen_3_t2: p3_t2 - 150 u3_t2 <= 0

 min_gen_1_t3: p1_t3 - 50 u1_t3 >= 0
 max_gen_1_t3: p1_t3 - 250 u1_t3 <= 0
 min_gen_2_t3: p2_t3 - 100 u2_t3 >= 0
 max_gen_2_t3: p2_t3 - 500 u2_t3 <= 0
 min_gen_3_t3: p3_t3 - 20 u3_t3 >= 0
 max_gen_3_t3: p3_t3 - 150 u3_t3 <= 0

Bounds
 0 <= u1_t1 <= 1
 0 <= u2_t1 <= 1
 0 <= u3_t1 <= 1
 0 <= u1_t2 <= 1
 0 <= u2_t2 <= 1
 0 <= u3_t2 <= 1
 0 <= u1_t3 <= 1
 0 <= u2_t3 <= 1
 0 <= u3_t3 <= 1
 p1_t1 >= 0
 p2_t1 >= 0
 p3_t1 >= 0
 p1_t2 >= 0
 p2_t2 >= 0
 p3_t2 >= 0
 p1_t3 >= 0
 p2_t3 >= 0
 p3_t3 >= 0

Binary
 u1_t1
 u2_t1
 u3_t1
 u1_t2
 u2_t2
 u3_t2
 u1_t3
 u2_t3
 u3_t3

End
