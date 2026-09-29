\ Badly Scaled LP Instance
\ Tests Ruiz Geometric Equilibrated Scaling on extreme coefficient magnitudes (1e-6 to 1e7)
Minimize
 obj: 1e6 x1 + 0.001 x2 + 500.0 x3 + 1e-5 x4

Subject To
 high_mag: 1e7 x1 + 50000 x2 + 1e6 x3 + 200 x4 >= 1e8
 low_mag: 1e-5 x1 + 2.5e-6 x2 + 1e-4 x3 + 5e-7 x4 >= 1e-3
 mixed_row: 1e4 x1 + 0.05 x2 + 2000 x3 + 0.001 x4 >= 5000

Bounds
 x1 >= 0
 x2 >= 0
 x3 >= 0
 x4 >= 0

End
