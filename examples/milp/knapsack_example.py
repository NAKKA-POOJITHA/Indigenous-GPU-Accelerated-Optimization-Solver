"""
Mixed-Integer Linear Programming (MILP) Example using Hunters Solver Python API
"""

import sys
import os

# Add python directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "python")))

from hunters_solver import Model, VarType

def main():
    print("==================================================")
    print("  Hunters Solver - MILP Python API Demonstration  ")
    print("==================================================")

    model = Model("KnapsackCapitalBudgeting")

    # Projects with NPV payoffs and Capital Costs
    # Project 1: Refinery CDU Debottlenecking  (NPV: $12M, Cost: $5M)
    # Project 2: High-Octane Reformer Upgrade  (NPV: $18M, Cost: $7M)
    # Project 3: Sulfur Recovery Unit-IV       (NPV: $25M, Cost: $11M)
    # Project 4: Green Hydrogen Electrolyzer  (NPV: $20M, Cost: $8M)
    # Project 5: Petrochemical Polypropylene  (NPV: $15M, Cost: $6M)

    p1 = model.add_variable("CDU_Debottleneck", binary=True, objective=12.0)
    p2 = model.add_variable("Reformer_Upgrade", binary=True, objective=18.0)
    p3 = model.add_variable("SRU_IV_Expansion", binary=True, objective=25.0)
    p4 = model.add_variable("Green_Hydrogen", binary=True, objective=20.0)
    p5 = model.add_variable("Petrochemical_PP", binary=True, objective=15.0)

    # Budget limit: Total investment <= $20M
    model.add_constraint(5*p1 + 7*p2 + 11*p3 + 8*p4 + 6*p5 <= 20.0, name="capital_budget")

    # Mutual exclusion / Strategic requirement: At least one clean fuel / environmental project
    model.add_constraint(p3 + p4 >= 1.0, name="clean_fuel_mandate")

    # Maximize total NPV
    model.maximize()

    print(f"Model: {model.name}")
    print(f"Variables: {len(model.variables)} binary decision variables")
    print(f"Constraints: {len(model.constraints)}")
    print("Solving via Hunters Branch-and-Bound Engine...")

    result = model.solve(verbose=True)

    print("\n--- Branch-and-Bound Solver Report ---")
    print(f"Status           : {result.status}")
    print(f"Optimal NPV      : ${result.objective:,.2f} Million")
    print(f"Best Bound       : ${result.best_bound:,.2f} Million")
    print(f"MIP Gap          : {result.mip_gap * 100:.2f}%")
    print(f"Explored Nodes   : {result.node_count}")
    print(f"Solve Time       : {result.solve_time_sec * 1000:.2f} ms")
    if result.validation:
        print(f"Solution Valid   : {result.validation.is_valid} [All Binary Constraints Satisfied]")
        print(f"Recalculated Obj : ${result.validation.recalculated_objective:,.2f} Million")

if __name__ == "__main__":
    main()
