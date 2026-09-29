# Mathematical Formulations & Optimization Algorithms
### Sovereign Mathematical Optimization Engine (Problem Statement 26119)

---

## 1. Linear Program Standard Form

Every linear program is transformed into the standard equality form:

$$\begin{aligned}
\min \quad & c^T x + c_0 \\
\text{s.t.} \quad & A x = b \\
& l \le x \le u
\end{aligned}$$

where:
- Inequality constraints $\sum_j a_{ij} x_j \le b_i$ are converted via slack variable $s_i \ge 0$: $\sum_j a_{ij} x_j + s_i = b_i$.
- Non-zero lower bounds $x_j \ge l_j$ are shifted: $x_j' = x_j - l_j \ge 0$, with RHS updated: $b' = b - A l$.

---

## 2. Revised Simplex Method

Let $A = [B \quad N]$ where $B$ is the $m \times m$ non-singular basis matrix.

1. **Primal Basic Solution**:
   $$x_B = B^{-1} b, \quad x_N = 0$$

2. **Simplex Multipliers (BTRAN)**:
   $$B^T \pi = c_B \implies \pi = B^{-T} c_B$$

3. **Reduced Costs (Pricing)**:
   $$d_j = c_j - \pi^T A_j \quad \forall j \in N$$

4. **Entering Variable Selection**:
   - **Dantzig Rule**: $q = \arg\min_j \{d_j : d_j < 0\}$.
   - **Steepest-Edge / Devex Rule**: $q = \arg\min_j \left\{ \frac{d_j}{\sqrt{\gamma_j}} : d_j < 0 \right\}$ where $\gamma_j \approx \|B^{-1} A_j\|_2^2$.
   - **Bland's Anti-Cycling Rule**: $q = \min \{ j \in N : d_j < -\epsilon \}$ (used when degeneracy or cycling is detected).

5. **FTRAN (Search Direction)**:
   $$B \alpha = A_q \implies \alpha = B^{-1} A_q$$

6. **Harris Two-Pass Ratio Test**:
   - **Pass 1**: Determine maximum feasible step $\theta_{\max}$:
     $$\theta_{\max} = \min_{i: \alpha_i > \epsilon} \frac{x_{B_i} + \delta_{feas}}{\alpha_i}$$
   - **Pass 2**: Among candidates with $\frac{x_{B_i}}{\alpha_i} \le \theta_{\max}$, choose pivot row $p$ with largest pivot element $|\alpha_p|$:
     $$p = \arg\max_{i \in \text{candidates}} \alpha_i$$

---

## 3. Matrix Equilibration (Ruiz Scaling)

To reduce numerical instability and ill-conditioning, Ruiz scaling iteratively balances row and column norms:

$$\begin{aligned}
R_i^{(k)} &= \frac{1}{\sqrt{\|A_{i, \cdot}^{(k)}\|_\infty}} \\
C_j^{(k)} &= \frac{1}{\sqrt{\|A_{\cdot, j}^{(k)}\|_\infty}} \\
A^{(k+1)} &= R^{(k)} A^{(k)} C^{(k)}
\end{aligned}$$

Iterated until $\max |1 - \|row_i\|_\infty| < 0.05$ or $k = 10$.

---

## 4. Branch-and-Bound Algorithm for MILP

Given MILP:
$$\min c^T x \quad \text{s.t.} \quad A x = b, \quad l \le x \le u, \quad x_j \in \mathbb{Z} \ \forall j \in I$$

1. **LP Relaxation**: Solve with continuous bounds.
2. **Integrality Check**: If $|x_j^* - \text{round}(x_j^*)| \le \epsilon_{int} \ \forall j \in I$, the relaxation is integer feasible.
3. **Branching Selection (Most Fractional Rule)**:
   $$j^* = \arg\max_{j \in I} \min(x_j^* - \lfloor x_j^* \rfloor, \lceil x_j^* \rceil - x_j^*)$$
4. **Node Splitting**:
   - Left Child: $x_{j^*} \le \lfloor x_{j^*}^* \rfloor$
   - Right Child: $x_{j^*} \ge \lceil x_{j^*}^* \rceil$
5. **MIP Optimality Gap**:
   $$\text{Gap} = \frac{|\text{BestBound} - \text{Incumbent}|}{\max(1, |\text{Incumbent}|)}$$
