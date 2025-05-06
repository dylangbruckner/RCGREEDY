#ifndef RCGREEDY_BASE_HPP
#define RCGREEDY_BASE_HPP
#include "event_generator.hpp"
#include <cmath>
#include <unordered_map>
#include <bitset>

class RCGREEDY {
    static constexpr int MAX_DEPTH = 10;  // maximum recursion depth of our scheduler 
    struct Group {
        size_t job_count = 0;
        size_t allocated_servers = 0;
        double percent_total_servers = 0.0;
        double average_p = 0.0;
        double p_min, p_max;  // todo do i even need these?
    };


    std::unordered_map<std::string, Group> groups;
    size_t server_count;
    int current_depth;

public:
    RCGREEDY(size_t servers, int max_depth);

private:
    // initalizes all map groups
    void initalize_groups();

    // recursive helper for initialize groups
    void generate_mappings(double c_p_min, double c_p_max, std::string c_string, int remaining_depth);
};




#endif // RCGREEDY_BASE_HPP