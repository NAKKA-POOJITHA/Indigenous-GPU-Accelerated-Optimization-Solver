"""
Model Representation for Hunters Optimization Solver
Supports Variables, Linear Expressions, Constraints, and LP file export.
"""

from enum import Enum
from typing import List, Dict, Union, Optional


class VarType(Enum):
    CONTINUOUS = "CONTINUOUS"
    INTEGER = "INTEGER"
    BINARY = "BINARY"


class ConstraintSense(Enum):
    LESS_EQUAL = "<="
    GREATER_EQUAL = ">="
    EQUAL = "="


class ObjectiveSense(Enum):
    MINIMIZE = "MINIMIZE"
    MAXIMIZE = "MAXIMIZE"


class LinearExpression:
    def __init__(self, terms: Optional[Dict['Variable', float]] = None, constant: float = 0.0):
        self.terms: Dict['Variable', float] = {}
        if terms:
            for v, c in terms.items():
                if abs(c) > 1e-15:
                    self.terms[v] = self.terms.get(v, 0.0) + c
        self.constant: float = constant

    def __add__(self, other: Union['LinearExpression', 'Variable', float, int]) -> 'LinearExpression':
        res = LinearExpression(self.terms, self.constant)
        if isinstance(other, (int, float)):
            res.constant += float(other)
        elif isinstance(other, Variable):
            res.terms[other] = res.terms.get(other, 0.0) + 1.0
        elif isinstance(other, LinearExpression):
            res.constant += other.constant
            for v, c in other.terms.items():
                res.terms[v] = res.terms.get(v, 0.0) + c
        return res

    def __radd__(self, other: Union['Variable', float, int]) -> 'LinearExpression':
        return self.__add__(other)

    def __sub__(self, other: Union['LinearExpression', 'Variable', float, int]) -> 'LinearExpression':
        res = LinearExpression(self.terms, self.constant)
        if isinstance(other, (int, float)):
            res.constant -= float(other)
        elif isinstance(other, Variable):
            res.terms[other] = res.terms.get(other, 0.0) - 1.0
        elif isinstance(other, LinearExpression):
            res.constant -= other.constant
            for v, c in other.terms.items():
                res.terms[v] = res.terms.get(v, 0.0) - c
        return res

    def __rsub__(self, other: Union['Variable', float, int]) -> 'LinearExpression':
        neg = self.__neg__()
        return neg.__add__(other)

    def __mul__(self, scalar: Union[float, int]) -> 'LinearExpression':
        scalar = float(scalar)
        res = LinearExpression({v: c * scalar for v, c in self.terms.items()}, self.constant * scalar)
        return res

    def __rmul__(self, scalar: Union[float, int]) -> 'LinearExpression':
        return self.__mul__(scalar)

    def __neg__(self) -> 'LinearExpression':
        return self.__mul__(-1.0)

    def __le__(self, other: Union['LinearExpression', 'Variable', float, int]) -> 'Constraint':
        expr = self - other
        # expr <= 0  => sum(coef * v) <= -constant
        return Constraint(
            name="",
            variables=list(expr.terms.keys()),
            coefficients=list(expr.terms.values()),
            sense=ConstraintSense.LESS_EQUAL,
            rhs=-expr.constant
        )

    def __ge__(self, other: Union['LinearExpression', 'Variable', float, int]) -> 'Constraint':
        expr = self - other
        return Constraint(
            name="",
            variables=list(expr.terms.keys()),
            coefficients=list(expr.terms.values()),
            sense=ConstraintSense.GREATER_EQUAL,
            rhs=-expr.constant
        )

    def __eq__(self, other: Union['LinearExpression', 'Variable', float, int]) -> 'Constraint':
        expr = self - other
        return Constraint(
            name="",
            variables=list(expr.terms.keys()),
            coefficients=list(expr.terms.values()),
            sense=ConstraintSense.EQUAL,
            rhs=-expr.constant
        )


class Variable:
    def __init__(self, name: str, lower_bound: float = 0.0, upper_bound: float = 1e30,
                 obj_coefficient: float = 0.0, var_type: VarType = VarType.CONTINUOUS, index: int = -1):
        self.name = name
        self.lower_bound = lower_bound
        self.upper_bound = upper_bound
        self.obj_coefficient = obj_coefficient
        self.var_type = var_type
        self.index = index

    def __add__(self, other):
        return LinearExpression({self: 1.0}) + other

    def __radd__(self, other):
        return LinearExpression({self: 1.0}) + other

    def __sub__(self, other):
        return LinearExpression({self: 1.0}) - other

    def __rsub__(self, other):
        return LinearExpression({self: -1.0}, float(other) if isinstance(other, (int, float)) else 0.0)

    def __mul__(self, other):
        return LinearExpression({self: float(other)})

    def __rmul__(self, other):
        return LinearExpression({self: float(other)})

    def __neg__(self):
        return LinearExpression({self: -1.0})

    def __le__(self, other):
        return LinearExpression({self: 1.0}) <= other

    def __ge__(self, other):
        return LinearExpression({self: 1.0}) >= other

    def __eq__(self, other):
        return LinearExpression({self: 1.0}) == other

    def __hash__(self):
        return hash(self.name)

    def __repr__(self):
        return f"Var({self.name})"


class Constraint:
    def __init__(self, name: str, variables: List[Variable], coefficients: List[float],
                 sense: ConstraintSense, rhs: float):
        self.name = name
        self.variables = variables
        self.coefficients = coefficients
        self.sense = sense
        self.rhs = rhs


class Model:
    def __init__(self, name: str = "HuntersModel"):
        self.name = name
        self.sense = ObjectiveSense.MINIMIZE
        self.obj_offset = 0.0
        self.variables: List[Variable] = []
        self.constraints: List[Constraint] = []
        self._var_map: Dict[str, Variable] = {}

    def add_variable(self, name: str, lower_bound: float = 0.0, upper_bound: float = 1e30,
                     objective: float = 0.0, var_type: Union[VarType, str] = VarType.CONTINUOUS,
                     integer: bool = False, binary: bool = False) -> Variable:
        if binary:
            v_type = VarType.BINARY
            lower_bound = 0.0
            upper_bound = 1.0
        elif integer:
            v_type = VarType.INTEGER
        elif isinstance(var_type, str):
            v_type = VarType[var_type.upper()]
        else:
            v_type = var_type

        idx = len(self.variables)
        var = Variable(name, lower_bound, upper_bound, objective, v_type, idx)
        self.variables.append(var)
        self._var_map[name] = var
        return var

    def add_constraint(self, constraint: Constraint, name: Optional[str] = None) -> Constraint:
        if name:
            constraint.name = name
        elif not constraint.name:
            constraint.name = f"c_{len(self.constraints) + 1}"
        self.constraints.append(constraint)
        return constraint

    def minimize(self, expr: Optional[LinearExpression] = None):
        self.sense = ObjectiveSense.MINIMIZE
        if expr is not None:
            self._set_objective(expr)

    def maximize(self, expr: Optional[LinearExpression] = None):
        self.sense = ObjectiveSense.MAXIMIZE
        if expr is not None:
            self._set_objective(expr)

    def _set_objective(self, expr: LinearExpression):
        for v in self.variables:
            v.obj_coefficient = 0.0
        for v, c in expr.terms.items():
            v.obj_coefficient = c
        self.obj_offset = expr.constant

    def to_lp_string(self) -> str:
        lines = []
        lines.append(f"\\ Model: {self.name}")
        lines.append("Maximize" if self.sense == ObjectiveSense.MAXIMIZE else "Minimize")

        # Objective
        obj_terms = []
        for v in self.variables:
            if abs(v.obj_coefficient) > 1e-15:
                obj_terms.append(f"{v.obj_coefficient:+.6g} {v.name}")
        if not obj_terms:
            obj_terms.append("0")
        lines.append(" obj: " + " ".join(obj_terms))

        lines.append("Subject To")
        for c in self.constraints:
            terms = []
            for v, coef in zip(c.variables, c.coefficients):
                terms.append(f"{coef:+.6g} {v.name}")
            sense_str = "<=" if c.sense == ConstraintSense.LESS_EQUAL else (">=" if c.sense == ConstraintSense.GREATER_EQUAL else "=")
            lines.append(f" {c.name}: {' '.join(terms)} {sense_str} {c.rhs:.6g}")

        lines.append("Bounds")
        for v in self.variables:
            if v.var_type == VarType.BINARY:
                lines.append(f" 0 <= {v.name} <= 1")
            else:
                lb_str = f"{v.lower_bound:.6g} <=" if v.lower_bound > -1e20 else ""
                ub_str = f"<= {v.upper_bound:.6g}" if v.upper_bound < 1e20 else ""
                if lb_str and ub_str:
                    lines.append(f" {lb_str} {v.name} {ub_str}")
                elif lb_str:
                    lines.append(f" {v.name} >= {v.lower_bound:.6g}")
                elif ub_str:
                    lines.append(f" {v.name} <= {v.upper_bound:.6g}")
                else:
                    lines.append(f" {v.name} free")

        binaries = [v.name for v in self.variables if v.var_type == VarType.BINARY]
        integers = [v.name for v in self.variables if v.var_type == VarType.INTEGER]

        if binaries:
            lines.append("Binary")
            for b in binaries:
                lines.append(f" {b}")

        if integers:
            lines.append("General")
            for ig in integers:
                lines.append(f" {ig}")

        lines.append("End")
        return "\n".join(lines)

    def write_lp(self, filepath: str):
        with open(filepath, "w") as f:
            f.write(self.to_lp_string())

    def solve(self, **kwargs) -> 'SolverResult':
        from .solver import HuntersSolver
        solver = HuntersSolver()
        return solver.solve(self, **kwargs)
