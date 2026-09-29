\ Capacitated Facility Location MILP
\ Minimize fixed facility setup cost + customer shipping freight
Minimize
 obj: 1200 y1 + 1500 y2 + 1100 y3 + 1800 y4 + 4 x11 + 6 x12 + 9 x13 + 3 x14 + 8 x15 + 7 x21 + 3 x22 + 5 x23 + 8 x24 + 4 x25 + 6 x31 + 8 x32 + 2 x33 + 5 x34 + 9 x35 + 2 x41 + 5 x42 + 7 x43 + 6 x44 + 3 x45

Subject To
 demand_c1: x11 + x21 + x31 + x41 = 80
 demand_c2: x12 + x22 + x32 + x42 = 120
 demand_c3: x13 + x23 + x33 + x43 = 90
 demand_c4: x14 + x24 + x34 + x44 = 110
 demand_c5: x15 + x25 + x35 + x45 = 100

 cap_fac_1: x11 + x12 + x13 + x14 + x15 - 250 y1 <= 0
 cap_fac_2: x21 + x22 + x23 + x24 + x25 - 300 y2 <= 0
 cap_fac_3: x31 + x32 + x33 + x34 + x35 - 200 y3 <= 0
 cap_fac_4: x41 + x42 + x43 + x44 + x45 - 350 y4 <= 0

Bounds
 0 <= y1 <= 1
 0 <= y2 <= 1
 0 <= y3 <= 1
 0 <= y4 <= 1
 x11 >= 0
 x12 >= 0
 x13 >= 0
 x14 >= 0
 x15 >= 0
 x21 >= 0
 x22 >= 0
 x23 >= 0
 x24 >= 0
 x25 >= 0
 x31 >= 0
 x32 >= 0
 x33 >= 0
 x34 >= 0
 x35 >= 0
 x41 >= 0
 x42 >= 0
 x43 >= 0
 x44 >= 0
 x45 >= 0

Binary
 y1
 y2
 y3
 y4

End
