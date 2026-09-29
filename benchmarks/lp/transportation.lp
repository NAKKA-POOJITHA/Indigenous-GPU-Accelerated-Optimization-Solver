\ Model: Transportation_Logistics_LP
\ Minimize multi-warehouse to multi-customer distribution freight cost
Minimize
 obj: 8 X11 + 14 X12 + 12 X13 + 17 X14 + 11 X21 + 9 X22 + 16 X23 + 13 X24 + 15 X31 + 12 X32 + 7 X33 + 10 X34

Subject To
 supply_warehouse_1: X11 + X12 + X13 + X14 <= 450
 supply_warehouse_2: X21 + X22 + X23 + X24 <= 600
 supply_warehouse_3: X31 + X32 + X33 + X34 <= 500

 demand_city_1: X11 + X21 + X31 >= 300
 demand_city_2: X12 + X22 + X32 >= 400
 demand_city_3: X13 + X23 + X33 >= 350
 demand_city_4: X14 + X24 + X34 >= 400

Bounds
 X11 >= 0
 X12 >= 0
 X13 >= 0
 X14 >= 0
 X21 >= 0
 X22 >= 0
 X23 >= 0
 X24 >= 0
 X31 >= 0
 X32 >= 0
 X33 >= 0
 X34 >= 0

End
