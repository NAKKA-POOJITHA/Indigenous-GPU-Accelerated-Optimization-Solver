\ Model: MRPL_Mangalore_Refinery_Blending_Scheduling
\ Objective: Maximize Net Daily Gross Refining Margin (GRM)
\ Indigenous Sovereign Solver Industrial Benchmark Instance
Maximize
 obj: 110.0 P_EuroVI_MS + 102.0 P_EuroVI_HSD + 98.0 P_ATF + 58.0 P_LPG + 42.0 P_Bitumen + 35.0 P_Petcoke - 74.5 C_Arabian_Light - 71.0 C_Arabian_Heavy - 76.8 C_Basrah_Medium - 82.5 C_Mumbai_High - 80.2 C_Sokol - 73.0 C_Maya - 2.5 F_CDU1 - 2.5 F_CDU2 - 3.2 F_CDU3 - 4.5 F_VDU - 12.0 F_CCR - 8.5 F_FCC - 14.0 F_HCU - 5.5 F_DHDT - 15000 U_CDU1 - 15000 U_CDU2 - 20000 U_CDU3 - 18000 U_VDU - 25000 U_CCR - 30000 U_FCC - 35000 U_HCU - 22000 U_DHDT

Subject To
 Supply_Arabian_Light: C_Arabian_Light <= 40000
 Supply_Arabian_Heavy: C_Arabian_Heavy <= 35000
 Supply_Basrah_Medium: C_Basrah_Medium <= 30000
 Supply_Mumbai_High: C_Mumbai_High <= 25000
 Supply_Sokol: C_Sokol <= 20000
 Supply_Maya: C_Maya <= 15000

 CDU_Feed_Sum: C_Arabian_Light + C_Arabian_Heavy + C_Basrah_Medium + C_Mumbai_High + C_Sokol + C_Maya - F_CDU1 - F_CDU2 - F_CDU3 = 0

 CDU1_Min: F_CDU1 - 10000 U_CDU1 >= 0
 CDU1_Max: F_CDU1 - 35000 U_CDU1 <= 0
 CDU2_Min: F_CDU2 - 10000 U_CDU2 >= 0
 CDU2_Max: F_CDU2 - 35000 U_CDU2 <= 0
 CDU3_Min: F_CDU3 - 15000 U_CDU3 >= 0
 CDU3_Max: F_CDU3 - 45000 U_CDU3 <= 0

 VDU_Min: F_VDU - 8000 U_VDU >= 0
 VDU_Max: F_VDU - 30000 U_VDU <= 0
 CCR_Min: F_CCR - 3000 U_CCR >= 0
 CCR_Max: F_CCR - 12000 U_CCR <= 0
 FCC_Min: F_FCC - 5000 U_FCC >= 0
 FCC_Max: F_FCC - 22000 U_FCC <= 0
 HCU_Min: F_HCU - 5000 U_HCU >= 0
 HCU_Max: F_HCU - 20000 U_HCU <= 0
 DHDT_Min: F_DHDT - 6000 U_DHDT >= 0
 DHDT_Max: F_DHDT - 25000 U_DHDT <= 0

 VDU_Balance: 0.38 C_Arabian_Light + 0.48 C_Arabian_Heavy + 0.45 C_Basrah_Medium + 0.28 C_Mumbai_High + 0.25 C_Sokol + 0.55 C_Maya - F_VDU = 0
 Naphtha_Balance: 0.18 C_Arabian_Light + 0.12 C_Arabian_Heavy + 0.14 C_Basrah_Medium + 0.22 C_Mumbai_High + 0.24 C_Sokol + 0.08 C_Maya - F_CCR - B_SRN_Gasoline = 0
 Gasoil_Balance: 0.28 C_Arabian_Light + 0.24 C_Arabian_Heavy + 0.25 C_Basrah_Medium + 0.32 C_Mumbai_High + 0.34 C_Sokol + 0.20 C_Maya - F_DHDT = 0
 VGO_Balance: 0.55 F_VDU - F_FCC - F_HCU = 0

 Gasoline_Pool: 0.88 F_CCR + 0.52 F_FCC + 0.20 F_HCU + B_SRN_Gasoline - P_EuroVI_MS = 0
 Diesel_Pool: 0.96 F_DHDT + 0.28 F_FCC + 0.72 F_HCU - P_EuroVI_HSD = 0
 ATF_Pool: 0.12 C_Arabian_Light + 0.08 C_Arabian_Heavy + 0.10 C_Basrah_Medium + 0.15 C_Mumbai_High + 0.14 C_Sokol + 0.05 C_Maya - P_ATF = 0
 LPG_Pool: 0.03 C_Arabian_Light + 0.02 C_Arabian_Heavy + 0.02 C_Basrah_Medium + 0.03 C_Mumbai_High + 0.03 C_Sokol + 0.02 C_Maya + 0.05 F_CCR + 0.08 F_FCC - P_LPG = 0
 Bitumen_Pool: 0.25 F_VDU - P_Bitumen = 0
 Petcoke_Pool: 0.15 F_VDU - P_Petcoke = 0

 Spec_Gasoline_RON: 102.0 F_CCR + 92.0 F_FCC + 85.0 F_HCU + 68.0 B_SRN_Gasoline - 95.0 P_EuroVI_MS >= 0
 Spec_Gasoline_Sulfur: 0.5 F_CCR + 15.0 F_FCC + 1.0 F_HCU + 80.0 B_SRN_Gasoline - 10.0 P_EuroVI_MS <= 0
 Spec_Diesel_Sulfur: 5.0 F_DHDT + 25.0 F_FCC + 2.0 F_HCU - 10.0 P_EuroVI_HSD <= 0
 Spec_Diesel_Cetane: 54.0 F_DHDT + 42.0 F_FCC + 58.0 F_HCU - 51.0 P_EuroVI_HSD >= 0

 Demand_Min_MS: P_EuroVI_MS >= 12000
 Demand_Max_MS: P_EuroVI_MS <= 30000
 Demand_Min_HSD: P_EuroVI_HSD >= 20000
 Demand_Max_HSD: P_EuroVI_HSD <= 50000
 Demand_Min_ATF: P_ATF >= 5000
 Demand_Max_ATF: P_ATF <= 15000
 Demand_Min_LPG: P_LPG >= 2000
 Demand_Max_LPG: P_LPG <= 8000

Bounds
 C_Arabian_Light >= 0
 C_Arabian_Heavy >= 0
 C_Basrah_Medium >= 0
 C_Mumbai_High >= 0
 C_Sokol >= 0
 C_Maya >= 0
 F_CDU1 >= 0
 F_CDU2 >= 0
 F_CDU3 >= 0
 F_VDU >= 0
 F_CCR >= 0
 F_FCC >= 0
 F_HCU >= 0
 F_DHDT >= 0
 B_SRN_Gasoline >= 0
 P_EuroVI_MS >= 0
 P_EuroVI_HSD >= 0
 P_ATF >= 0
 P_LPG >= 0
 P_Bitumen >= 0
 P_Petcoke >= 0
 0 <= U_CDU1 <= 1
 0 <= U_CDU2 <= 1
 0 <= U_CDU3 <= 1
 0 <= U_VDU <= 1
 0 <= U_CCR <= 1
 0 <= U_FCC <= 1
 0 <= U_HCU <= 1
 0 <= U_DHDT <= 1

Binary
 U_CDU1
 U_CDU2
 U_CDU3
 U_VDU
 U_CCR
 U_FCC
 U_HCU
 U_DHDT

End
