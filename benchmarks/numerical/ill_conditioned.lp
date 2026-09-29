\ Ill-Conditioned Linear Program
\ Tests LU condition number estimation, dynamic regularized pivoting, and residual check
Minimize
 obj: 2.0 x1 + 2.000001 x2 + 3.0 x3

Subject To
 c1: 1.00000 x1 + 1.000000 x2 + 0.5 x3 >= 10.0
 c2: 1.00000 x1 + 1.000001 x2 + 0.500001 x3 >= 10.00001
 c3: 0.2 x1 + 0.3 x2 + 1.0 x3 >= 4.0

Bounds
 x1 >= 0
 x2 >= 0
 x3 >= 0

End
