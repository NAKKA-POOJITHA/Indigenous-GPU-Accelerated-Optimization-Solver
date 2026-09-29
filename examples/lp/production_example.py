"""
Linear Programming Example using Hunters Solver Python API
"""

import sys
import os

# Add python directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "python")))

from hunters_solver import Model, VarType

def main():
    print("==================================================")
    print("  Hunters Solver - Linear Programming Python API  ")
    print("==================================================")

    model = Model("FactoryProductionLP")

    # Add variables (Continuous)
    x = model.add_variable("Chairs", lower_bound=0, upper_bound=1000, objective=45.0)
    y = model.add_variable("Tables", lower_bound=0, upper_bound=500, objective=80.0)
    z = model.add_variable("Desks", lower_bound=0, upper_bound=300, objective=110.0)

    # Add constraints
    # Wood: 5*x + 20*y + 15*z <= 4000
    model.add_constraint(5*x + 20*y + 15*z <= 4000, name="wood_capacity")

    # Labor: 2*x + 5*y + 8*z <= 1200
    model.add_constraint(2*x + 5*y + 8*z <= 1200, name="labor_capacity")

    # Machine hours: 1.5*x + 3*y + 4*z <= 800
    model.add_constraint(1.5*x + 3*y + 4*z <= 800, name="machine_hours")

    # Objective: Maximize Profit
    model.maximize()

    print(f"Model created with {len(model.variables)} variables and {len(model.constraints)} constraints.")
    print("Solving via Hunters Core Engine...")

    result = model.solve(verbose=True)

    print("\n--- Solver Result ---")
    print(f"Status           : {result.status}")
    print(f"Optimal Profit   : Rs. {result.objective:,.2f}")
    print(f"Solve Time       : {result.solve_time_sec*1000:.2f} ms")
    if result.validation:
        print(f"Solution Valid   : {result.validation.is_valid}")
        print(f"Recalculated Obj : Rs. {result.validation.recalculated_objective:,.2f}")

if __name__ == "__main__":
    main()
