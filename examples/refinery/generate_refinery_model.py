#!/usr/bin/env python3
"""
MRPL Industrial Crude Blending & Refinery Production Planning Model Generator
Problem Statement 26119: Indigenous GPU-Accelerated Optimization Solver
"""

import os
import sys

def generate_refinery_lp(output_path="examples/refinery/mrpl_crude_blending.lp"):
    lp_lines = []
    lp_lines.append("\\ MRPL Refinery Crude Blending and Production Scheduling Model")
    lp_lines.append("\\ Mangalore Refinery and Petrochemicals Limited (300,000 bpd capacity)")
    lp_lines.append("\\ Maximizes Gross Refining Margin (GRM) in USD/day\n")

    # Objective: Maximize Revenue - Crude Cost - Operating/Hydrogen Costs
    lp_lines.append("Maximize")
    lp_lines.append("  obj: "
                    # Product Revenues ($/bbl)
                    "+ 102.0 ms_regular + 112.0 ms_premium + 98.0 hsd_diesel + 108.0 atf_jet "
                    "+ 65.0 prod_lpg + 55.0 prod_fuel_oil + 62.0 prod_bitumen "
                    # Crude Costs ($/bbl)
                    "- 78.0 c_arab_light - 72.0 c_arab_heavy - 82.0 c_bonny_light - 68.0 c_maya - 74.0 c_basrah "
                    # Operating / Utility Costs ($/bbl processed)
                    "- 2.50 u_cdu - 3.20 u_vdu - 5.50 u_ccr - 6.20 u_fcc - 7.00 u_hcu - 3.80 u_dhdt "
                    # Mode Switch / Tank Setup Costs
                    "- 5000.0 z_hcu_diesel - 4500.0 z_hcu_jet "
                    "- 2000.0 y_tank_al - 2000.0 y_tank_ah - 2000.0 y_tank_bl - 2000.0 y_tank_my - 2000.0 y_tank_bm")

    lp_lines.append("\nSubject To")

    # 1. Total Crude Distillation Unit (CDU) Mass Balance & Capacity
    lp_lines.append("  c_cdu_balance: c_arab_light + c_arab_heavy + c_bonny_light + c_maya + c_basrah - u_cdu = 0")
    lp_lines.append("  c_cdu_max_cap: u_cdu <= 300000")
    lp_lines.append("  c_cdu_min_cap: u_cdu >= 150000")

    # 2. Crude Availability Limits
    lp_lines.append("  c_avail_al: c_arab_light <= 120000")
    lp_lines.append("  c_avail_ah: c_arab_heavy <= 100000")
    lp_lines.append("  c_avail_bl: c_bonny_light <= 80000")
    lp_lines.append("  c_avail_my: c_maya <= 70000")
    lp_lines.append("  c_avail_bm: c_basrah <= 90000")

    # 3. Semi-continuous Parcel Shipment Rules (Binary linkage)
    lp_lines.append("  c_link_al_ub: c_arab_light - 120000 y_tank_al <= 0")
    lp_lines.append("  c_link_al_lb: c_arab_light - 20000 y_tank_al >= 0")
    lp_lines.append("  c_link_ah_ub: c_arab_heavy - 100000 y_tank_ah <= 0")
    lp_lines.append("  c_link_ah_lb: c_arab_heavy - 20000 y_tank_ah >= 0")
    lp_lines.append("  c_link_bl_ub: c_bonny_light - 80000 y_tank_bl <= 0")
    lp_lines.append("  c_link_bl_lb: c_bonny_light - 20000 y_tank_bl >= 0")
    lp_lines.append("  c_link_my_ub: c_maya - 70000 y_tank_my <= 0")
    lp_lines.append("  c_link_my_lb: c_maya - 20000 y_tank_my >= 0")
    lp_lines.append("  c_link_bm_ub: c_basrah - 90000 y_tank_bm <= 0")
    lp_lines.append("  c_link_bm_lb: c_basrah - 20000 y_tank_bm >= 0")

    # 4. CDU Straight-Run Cuts Yields
    # LPG: AL=0.03, AH=0.02, BL=0.04, MY=0.015, BM=0.025
    lp_lines.append("  c_yield_lpg: 0.03 c_arab_light + 0.02 c_arab_heavy + 0.04 c_bonny_light + 0.015 c_maya + 0.025 c_basrah - stream_sr_lpg = 0")
    # Light Naphtha: AL=0.08, AH=0.05, BL=0.12, MY=0.04, BM=0.06
    lp_lines.append("  c_yield_ln: 0.08 c_arab_light + 0.05 c_arab_heavy + 0.12 c_bonny_light + 0.04 c_maya + 0.06 c_basrah - stream_sr_ln = 0")
    # Heavy Naphtha: AL=0.14, AH=0.10, BL=0.18, MY=0.08, BM=0.12
    lp_lines.append("  c_yield_hn: 0.14 c_arab_light + 0.10 c_arab_heavy + 0.18 c_bonny_light + 0.08 c_maya + 0.12 c_basrah - stream_sr_hn = 0")
    # Kerosene: AL=0.10, AH=0.08, BL=0.12, MY=0.06, BM=0.09
    lp_lines.append("  c_yield_kero: 0.10 c_arab_light + 0.08 c_arab_heavy + 0.12 c_bonny_light + 0.06 c_maya + 0.09 c_basrah - stream_sr_kero = 0")
    # Gasoil: AL=0.22, AH=0.20, BL=0.25, MY=0.18, BM=0.21
    lp_lines.append("  c_yield_go: 0.22 c_arab_light + 0.20 c_arab_heavy + 0.25 c_bonny_light + 0.18 c_maya + 0.21 c_basrah - stream_sr_go = 0")
    # Atmospheric Residue: AL=0.43, AH=0.55, BL=0.29, MY=0.625, BM=0.495
    lp_lines.append("  c_yield_ar: 0.43 c_arab_light + 0.55 c_arab_heavy + 0.29 c_bonny_light + 0.625 c_maya + 0.495 c_basrah - stream_sr_ar = 0")

    # 5. VDU Processing and Residue Split
    lp_lines.append("  c_vdu_feed: stream_sr_ar - u_vdu - stream_ar_to_fo = 0")
    lp_lines.append("  c_vdu_cap: u_vdu <= 130000")
    # VDU yields: 0.55 VGO, 0.45 Vacuum Residue (VR)
    lp_lines.append("  c_vdu_vgo: 0.55 u_vdu - stream_vgo = 0")
    lp_lines.append("  c_vdu_vr: 0.45 u_vdu - stream_vr = 0")
    lp_lines.append("  c_vr_split: stream_vr - prod_bitumen - stream_vr_to_fo = 0")

    # 6. CCR (Reformer) - Upgrades Heavy Naphtha to Reformate
    lp_lines.append("  c_ccr_feed: stream_sr_hn - u_ccr - stream_hn_to_petrol = 0")
    lp_lines.append("  c_ccr_cap: u_ccr <= 45000")
    # Reformer yield: 0.88 Reformate (RON 100), 0.08 LPG, 0.04 Fuel Gas
    lp_lines.append("  c_ccr_reformate: 0.88 u_ccr - stream_reformate = 0")
    lp_lines.append("  c_ccr_lpg: 0.08 u_ccr - stream_ccr_lpg = 0")

    # 7. VGO Splitting between FCC and HCU
    lp_lines.append("  c_vgo_split: stream_vgo - u_fcc - u_hcu = 0")
    lp_lines.append("  c_fcc_cap: u_fcc <= 55000")
    lp_lines.append("  c_hcu_cap: u_hcu <= 65000")

    # FCC Yields: 0.15 LPG, 0.50 FCC Gasoline (RON 92), 0.25 LCO (to Diesel), 0.10 Slurry
    lp_lines.append("  c_fcc_lpg: 0.15 u_fcc - stream_fcc_lpg = 0")
    lp_lines.append("  c_fcc_gas: 0.50 u_fcc - stream_fcc_gas = 0")
    lp_lines.append("  c_fcc_lco: 0.25 u_fcc - stream_fcc_lco = 0")
    lp_lines.append("  c_fcc_slurry: 0.10 u_fcc - stream_fcc_slurry = 0")

    # HCU Mode Selection (Binary Mutex)
    lp_lines.append("  c_hcu_mode_select: z_hcu_diesel + z_hcu_jet = 1")
    # HCU Yields depend on operating mode:
    # Mode 1 (Max Diesel): 0.65 HCU Diesel, 0.20 HCU Kero, 0.15 Naphtha/LPG
    # Mode 2 (Max Jet): 0.35 HCU Diesel, 0.50 HCU Kero, 0.15 Naphtha/LPG
    lp_lines.append("  c_hcu_diesel_yield: stream_hcu_diesel - 0.65 u_hcu - 10000 z_hcu_diesel <= 0")
    lp_lines.append("  c_hcu_diesel_yield_b: stream_hcu_diesel - 0.35 u_hcu <= 0")
    lp_lines.append("  c_hcu_jet_yield: stream_hcu_kero - 0.50 u_hcu - 10000 z_hcu_jet <= 0")
    lp_lines.append("  c_hcu_jet_yield_b: stream_hcu_kero - 0.20 u_hcu <= 0")

    # 8. Diesel Hydrotreater (DHDT)
    lp_lines.append("  c_dhdt_feed: stream_sr_go + stream_fcc_lco - u_dhdt = 0")
    lp_lines.append("  c_dhdt_cap: u_dhdt <= 75000")
    lp_lines.append("  c_dhdt_out: 0.98 u_dhdt - stream_dhdt_diesel = 0")

    # 9. Product Blending Pools
    # LPG Pool
    lp_lines.append("  c_lpg_pool: stream_sr_lpg + stream_ccr_lpg + stream_fcc_lpg - prod_lpg = 0")

    # Petrol / Gasoline Blending Pools (Regular RON 91 vs Premium RON 95)
    # Blending components: Light Naphtha (RON 70), Heavy Naphtha (RON 60), Reformate (RON 100), FCC Gasoline (RON 92)
    lp_lines.append("  c_ms_reg_split: ms_reg_ln + ms_reg_hn + ms_reg_ref + ms_reg_fcc - ms_regular = 0")
    lp_lines.append("  c_ms_prem_split: ms_prem_ln + ms_prem_hn + ms_prem_ref + ms_prem_fcc - ms_premium = 0")
    lp_lines.append("  c_ln_pool: ms_reg_ln + ms_prem_ln - stream_sr_ln <= 0")
    lp_lines.append("  c_hn_pool: ms_reg_hn + ms_prem_hn - stream_hn_to_petrol <= 0")
    lp_lines.append("  c_ref_pool: ms_reg_ref + ms_prem_ref - stream_reformate <= 0")
    lp_lines.append("  c_fcc_gas_pool: ms_reg_fcc + ms_prem_fcc - stream_fcc_gas <= 0")

    # Octane Quality Constraints:
    # Regular: 70*LN + 60*HN + 100*Ref + 92*FCC >= 91 * MS_regular
    lp_lines.append("  c_octane_reg: 70.0 ms_reg_ln + 60.0 ms_reg_hn + 100.0 ms_reg_ref + 92.0 ms_reg_fcc - 91.0 ms_regular >= 0")
    # Premium: 70*LN + 60*HN + 100*Ref + 92*FCC >= 95 * MS_premium
    lp_lines.append("  c_octane_prem: 70.0 ms_prem_ln + 60.0 ms_prem_hn + 100.0 ms_prem_ref + 92.0 ms_prem_fcc - 95.0 ms_premium >= 0")

    # Diesel (HSD) Pool
    lp_lines.append("  c_diesel_pool: stream_dhdt_diesel + stream_hcu_diesel - hsd_diesel = 0")

    # ATF / Jet Fuel Pool
    lp_lines.append("  c_atf_pool: stream_sr_kero + stream_hcu_kero - atf_jet = 0")

    # Fuel Oil Pool
    lp_lines.append("  c_fo_pool: stream_ar_to_fo + stream_vr_to_fo + stream_fcc_slurry - prod_fuel_oil = 0")

    # 10. Minimum Market Demands & Commitments
    lp_lines.append("  c_dem_ms_reg: ms_regular >= 25000")
    lp_lines.append("  c_dem_ms_prem: ms_premium >= 10000")
    lp_lines.append("  c_dem_diesel: hsd_diesel >= 60000")
    lp_lines.append("  c_dem_atf: atf_jet >= 20000")
    lp_lines.append("  c_dem_lpg: prod_lpg >= 8000")
    lp_lines.append("  c_dem_bitumen: prod_bitumen >= 5000")

    # Bounds
    lp_lines.append("\nBounds")
    lp_lines.append("  0 <= ms_regular <= 100000")
    lp_lines.append("  0 <= ms_premium <= 60000")
    lp_lines.append("  0 <= hsd_diesel <= 150000")
    lp_lines.append("  0 <= atf_jet <= 80000")
    lp_lines.append("  0 <= prod_lpg <= 40000")
    lp_lines.append("  0 <= prod_fuel_oil <= 50000")
    lp_lines.append("  0 <= prod_bitumen <= 30000")

    # Binaries
    lp_lines.append("\nBinaries")
    lp_lines.append("  y_tank_al y_tank_ah y_tank_bl y_tank_my y_tank_bm")
    lp_lines.append("  z_hcu_diesel z_hcu_jet")

    lp_lines.append("\nEnd")

    content = "\n".join(lp_lines)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w") as f:
        f.write(content)
    print(f"[MRPL Model Generator] Successfully generated {output_path} ({len(lp_lines)} lines)")
    return output_path

if __name__ == "__main__":
    generate_refinery_lp()
