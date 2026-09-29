"""
Solver Execution & Result Parsing for Hunters Optimization Solver
"""

import subprocess
import tempfile
import os
import re
from dataclasses import dataclass, field
from typing import List, Dict, Optional
from .model import Model


@dataclass
class ValidationReport:
    is_valid: bool = False
    max_primal_residual: float = 0.0
    max_bound_violation: float = 0.0
    max_integrality_violation: float = 0.0
    recalculated_objective: float = 0.0
    objective_discrepancy: float = 0.0
    details: str = ""


@dataclass
class SolverResult:
    status: str = "UNKNOWN"
    objective: float = 0.0
    best_bound: float = 0.0
    mip_gap: float = 0.0
    solve_time_sec: float = 0.0
    node_count: int = 0
    lp_iterations: int = 0
    solution: Dict[str, float] = field(default_factory=dict)
    validation: Optional[ValidationReport] = None
    raw_output: str = ""


@dataclass
class SolverOptions:
    solver: str = "auto"          # auto, lp, milp
    method: str = "simplex"       # simplex
    node_selection: str = "best-bound"  # best-bound, depth-first
    branching: str = "most-fractional"  # most-fractional, first-fractional
    presolve: bool = True
    scaling: bool = True
    gpu: bool = False
    heuristics: bool = True
    time_limit: float = 300.0
    mip_gap: float = 1e-4
    verbose: bool = False


class HuntersSolver:
    def __init__(self, binary_path: Optional[str] = None):
        if binary_path is None:
            # Look in build directory relative to this package or workspace
            possible_paths = [
                os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "build", "hunters-solver.exe")),
                os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "build", "hunters-solver")),
                os.path.abspath(os.path.join(os.getcwd(), "build", "hunters-solver.exe")),
                os.path.abspath(os.path.join(os.getcwd(), "build", "hunters-solver")),
                "hunters-solver.exe",
                "hunters-solver"
            ]
            for p in possible_paths:
                if os.path.isfile(p):
                    self.binary_path = p
                    break
            else:
                self.binary_path = possible_paths[0]
        else:
            self.binary_path = binary_path

    def solve_file(self, lp_filepath: str, options: Optional[SolverOptions] = None, **kwargs) -> SolverResult:
        if options is None:
            options = SolverOptions()

        for k, v in kwargs.items():
            if hasattr(options, k):
                setattr(options, k, v)

        cmd = [self.binary_path, lp_filepath]
        if options.solver != "auto":
            cmd.extend(["--solver", options.solver])
        if options.method:
            cmd.extend(["--method", options.method])
        if options.node_selection:
            cmd.extend(["--node-selection", options.node_selection])
        if options.branching:
            cmd.extend(["--branching", options.branching])
        cmd.extend(["--presolve", "on" if options.presolve else "off"])
        cmd.extend(["--scaling", "on" if options.scaling else "off"])
        cmd.extend(["--gpu", "on" if options.gpu else "off"])
        cmd.extend(["--heuristics", "on" if options.heuristics else "off"])
        cmd.extend(["--time-limit", str(options.time_limit)])
        cmd.extend(["--mip-gap", str(options.mip_gap)])
        if options.verbose:
            cmd.append("--verbose")

        try:
            res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
            output = res.stdout + "\n" + res.stderr
            return self._parse_output(output)
        except Exception as e:
            result = SolverResult()
            result.status = "ERROR"
            result.raw_output = f"Execution error: {str(e)}"
            return result

    def solve(self, model: Model, options: Optional[SolverOptions] = None, **kwargs) -> SolverResult:
        with tempfile.NamedTemporaryFile(suffix=".lp", delete=False, mode="w") as f:
            f.write(model.to_lp_string())
            temp_path = f.name

        try:
            return self.solve_file(temp_path, options, **kwargs)
        finally:
            if os.path.exists(temp_path):
                os.remove(temp_path)

    def _parse_output(self, output: str) -> SolverResult:
        res = SolverResult(raw_output=output)

        # Parse Incumbent / Objective
        obj_match = re.search(r"Incumbent\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
        if obj_match:
            res.objective = float(obj_match.group(1))
        else:
            obj_lp_match = re.search(r"Objective\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
            if obj_lp_match:
                res.objective = float(obj_lp_match.group(1))

        # Best bound
        bb_match = re.search(r"Best Bound\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
        if bb_match:
            res.best_bound = float(bb_match.group(1))

        # MIP gap
        gap_match = re.search(r"MIP Gap\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*%", output)
        if gap_match:
            res.mip_gap = float(gap_match.group(1)) / 100.0

        # Solve time
        time_match = re.search(r"Total solve time\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*s", output)
        if time_match:
            res.solve_time_sec = float(time_match.group(1))

        # Node count
        node_match = re.search(r"Nodes\s*:\s*(\d+)", output)
        if node_match:
            res.node_count = int(node_match.group(1))

        # LP iterations
        iter_match = re.search(r"LP Iteration\s*:\s*(\d+)", output)
        if iter_match:
            res.lp_iterations = int(iter_match.group(1))

        # Status
        status_match = re.search(r"Solution Status\s*:\s*([A-Za-z_ /]+)", output)
        if status_match:
            res.status = status_match.group(1).strip()
        elif "VALID SOLUTION [PASSED]" in output:
            res.status = "OPTIMAL"

        # Validation report
        val = ValidationReport()
        if "VALID SOLUTION [PASSED]" in output:
            val.is_valid = True
        elif "SOLUTION VALIDATION FAILED" in output:
            val.is_valid = False

        res_inf = re.search(r"Max Primal Residual\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
        if res_inf:
            val.max_primal_residual = float(res_inf.group(1))

        bnd_inf = re.search(r"Max Bound Violation\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
        if bnd_inf:
            val.max_bound_violation = float(bnd_inf.group(1))

        int_inf = re.search(r"Max Integrality Viol\.\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
        if int_inf:
            val.max_integrality_violation = float(int_inf.group(1))

        rec_obj = re.search(r"Recalculated Obj\s*:\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", output)
        if rec_obj:
            val.recalculated_objective = float(rec_obj.group(1))

        res.validation = val
        return res
