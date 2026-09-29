#include "model/Model.hpp"
#include <iostream>
#include <iomanip>

namespace hunters {

void Model::print_summary() const {
    std::cout << "================ MODEL SUMMARY ================" << std::endl;
    std::cout << "Model Name      : " << name << std::endl;
    std::cout << "Objective Sense : " << (sense == ObjectiveSense::MINIMIZE ? "MINIMIZE" : "MAXIMIZE") << std::endl;
    std::cout << "Variables       : " << variables.size() << std::endl;
    std::cout << "  Continuous    : " << (variables.size() - num_integer_variables()) << std::endl;
    std::cout << "  Integer/Binary: " << num_integer_variables() << std::endl;
    std::cout << "Constraints     : " << constraints.size() << std::endl;
    std::cout << "Nonzeros (NNZ)  : " << num_nonzeros() << std::endl;
    std::cout << "Problem Type    : " << (is_milp() ? "MILP" : "LP") << std::endl;
    std::cout << "===============================================" << std::endl;
}

} // namespace hunters
