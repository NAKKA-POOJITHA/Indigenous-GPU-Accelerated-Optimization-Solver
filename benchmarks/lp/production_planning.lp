\ Model: Production_Planning_LP
\ Multi-product manufacturing LP with raw material and machining constraints
Maximize
 obj: 120 Prod_A + 160 Prod_B + 210 Prod_C + 280 Prod_D + 95 Prod_E

Subject To
 raw_steel: 3.5 Prod_A + 4.0 Prod_B + 6.0 Prod_C + 8.5 Prod_D + 2.0 Prod_E <= 4500
 raw_aluminum: 2.0 Prod_A + 5.5 Prod_B + 3.0 Prod_C + 6.0 Prod_D + 4.5 Prod_E <= 3800
 machining_hours: 1.5 Prod_A + 2.0 Prod_B + 3.5 Prod_C + 4.0 Prod_D + 1.2 Prod_E <= 2400
 assembly_hours: 2.5 Prod_A + 3.0 Prod_B + 4.0 Prod_C + 5.5 Prod_D + 2.0 Prod_E <= 3200
 quality_inspection: 0.5 Prod_A + 0.8 Prod_B + 1.2 Prod_C + 1.5 Prod_D + 0.4 Prod_E <= 800
 packaging_capacity: Prod_A + Prod_B + Prod_C + Prod_D + Prod_E <= 1500
 min_contract_A: Prod_A >= 100
 min_contract_B: Prod_B >= 150

Bounds
 100 <= Prod_A <= 600
 150 <= Prod_B <= 500
 0 <= Prod_C <= 400
 0 <= Prod_D <= 300
 0 <= Prod_E <= 800

End
