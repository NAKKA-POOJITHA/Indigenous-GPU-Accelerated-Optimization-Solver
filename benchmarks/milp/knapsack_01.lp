\ 0-1 Knapsack Benchmark
\ Multi-item high density knapsack problem
Maximize
 obj: 42 x1 + 38 x2 + 55 x3 + 47 x4 + 32 x5 + 68 x6 + 45 x7 + 51 x8 + 36 x9 + 60 x10 + 28 x11 + 72 x12

Subject To
 weight_limit: 12 x1 + 10 x2 + 15 x3 + 13 x4 + 9 x5 + 20 x6 + 14 x7 + 16 x8 + 11 x9 + 18 x10 + 8 x11 + 22 x12 <= 65
 volume_limit: 8 x1 + 7 x2 + 11 x3 + 9 x4 + 6 x5 + 14 x6 + 10 x7 + 12 x8 + 8 x9 + 13 x10 + 5 x11 + 16 x12 <= 48

Bounds
 0 <= x1 <= 1
 0 <= x2 <= 1
 0 <= x3 <= 1
 0 <= x4 <= 1
 0 <= x5 <= 1
 0 <= x6 <= 1
 0 <= x7 <= 1
 0 <= x8 <= 1
 0 <= x9 <= 1
 0 <= x10 <= 1
 0 <= x11 <= 1
 0 <= x12 <= 1

Binary
 x1
 x2
 x3
 x4
 x5
 x6
 x7
 x8
 x9
 x10
 x11
 x12

End
