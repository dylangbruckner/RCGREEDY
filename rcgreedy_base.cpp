#include "rcgreedy_base.hpp"


RCGREEDY::RCGREEDY(size_t servers, int max_depth) : server_count(servers), current_depth(std::max(1, std::min(max_depth, MAX_DEPTH))) {

}

void RCGREEDY::initalize_groups(){
    generate_mappings(0.0, 1.0, "", current_depth);
}

// recursive helper for initialize groups
void RCGREEDY::generate_mappings(double c_p_min, double c_p_max, std::string c_string, int remaining_depth){
    double diff = (c_p_max - c_p_min) / 2;

    // add new mappings
    groups[c_string + "1"] = Group{0, 0, 0.0, 0.0, c_p_min + diff, c_p_max};
    groups[c_string + "0"] = Group{0, 0, 0.0, 0.0, c_p_min, c_p_max - diff};

    // add new mappings for longer strings
    if (remaining_depth > 1) {
        generate_mappings(c_p_min + diff, c_p_max, c_string + "1", remaining_depth - 1);
        generate_mappings(c_p_min, c_p_max - diff, c_string + "0", remaining_depth - 1);
    }

}