"""
Hunters Indigenous Optimization Solver
Sovereign Mathematical Optimization Engine (LP / MILP)
Problem Statement 26119 (MRPL)
"""

from .model import Model, Variable, Constraint, VarType, ConstraintSense, ObjectiveSense
from .solver import HuntersSolver, SolverResult, SolverOptions, ValidationReport

__version__ = "0.1.0"
__all__ = [
    "Model",
    "Variable",
    "Constraint",
    "VarType",
    "ConstraintSense",
    "ObjectiveSense",
    "HuntersSolver",
    "SolverResult",
    "SolverOptions",
    "ValidationReport",
]
