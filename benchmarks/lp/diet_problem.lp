\ Model: Stigler_Diet_LP
\ Optimal nutrient cost minimization
Minimize
 obj: 1.84 Bread + 2.19 Milk + 3.49 Cheese + 4.99 Chicken + 0.79 Apples + 1.29 Rice + 2.49 Oats + 3.19 Eggs

Subject To
 calories: 110 Bread + 150 Milk + 320 Cheese + 220 Chicken + 80 Apples + 130 Rice + 150 Oats + 75 Eggs >= 2200
 protein: 4.0 Bread + 8.0 Milk + 20.0 Cheese + 28.0 Chicken + 0.5 Apples + 2.5 Rice + 5.0 Oats + 6.0 Eggs >= 65
 calcium: 25 Bread + 290 Milk + 600 Cheese + 15 Chicken + 10 Apples + 10 Rice + 20 Oats + 25 Eggs >= 1000
 iron: 0.9 Bread + 0.1 Milk + 0.2 Cheese + 1.2 Chicken + 0.2 Apples + 1.8 Rice + 2.0 Oats + 0.9 Eggs >= 18
 vit_a: 0 Bread + 500 Milk + 1000 Cheese + 80 Chicken + 100 Apples + 0 Rice + 0 Oats + 260 Eggs >= 3000
 max_fat: 1.0 Bread + 8.0 Milk + 25.0 Cheese + 10.0 Chicken + 0.2 Apples + 0.3 Rice + 3.0 Oats + 5.0 Eggs <= 70

Bounds
 Bread >= 0
 Milk >= 0
 Cheese >= 0
 Chicken >= 0
 Apples >= 0
 Rice >= 0
 Oats >= 0
 Eggs >= 0

End
