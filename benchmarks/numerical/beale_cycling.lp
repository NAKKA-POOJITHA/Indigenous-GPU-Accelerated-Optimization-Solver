\ Beale's Degenerate Cycling LP
\ Naive Dantzig pivot rule cycles endlessly without Bland's anti-cycling rule
Maximize
 obj: 10 x1 - 57 x2 - 9 x3 - 24 x4

Subject To
 c1: 0.5 x1 - 5.5 x2 - 2.5 x3 + 9.0 x4 <= 0
 c2: 0.5 x1 - 1.5 x2 - 0.5 x3 + 1.0 x4 <= 0
 c3: x1 <= 1

Bounds
 x1 >= 0
 x2 >= 0
 x3 >= 0
 x4 >= 0

End
