#include "rcgreedy_base.hpp"


RCGREEDY::RCGREEDY(size_t servers, int max_depth, double average_size, bool partial_server_allocs = false) : 
    server_count(servers), current_depth(std::max(0, std::min(max_depth, RCGREEDY::MAX_DEPTH))), 
    maximization_constant(1/average_size), partial_servers(partial_server_allocs) {
    initalize_groups();

    // initally, give all of servers to the top group
    groups[""].allocated_servers = server_count;
}

void RCGREEDY::full_realloc() {
    max_update += 1;
    history.clear(); // new action, remake history vector
    if (!groups[""].job_count) return;
    partial_realloc("");
}

void RCGREEDY::add_job(RCGREEDY_Job &job, bool forced_local_realloc) {
    if (job_group_assignments.find(job) != job_group_assignments.end()) {
        std::cerr << "Error adding job " << job.id << ". Job already exists" << std::endl;
        return; 
    }

    std::string group = get_group_id(job);
    id_to_jobs[group].insert(job); // add job to group list
    std::string last_level_w_servers = ""; // used for local realloc
    std::string c_level;
    size_t current_update = groups[""].update_count;
    history.clear(); // new action, remake history vector

    // find highest level where it is the only job
    for (size_t len = 0; len <= group.size(); ++len) {
        c_level = group.substr(0, len);

        // check if group information is updated
        if (groups[c_level].update_count >= current_update) {
            current_update = groups[c_level].update_count;

            // if servers found, update for local realloc
            if (groups[c_level].allocated_servers) {
                last_level_w_servers = c_level;
            }

            
        } else {
            // udpate group information
            groups[c_level].update_count = current_update;
            groups[c_level].allocated_servers = 0;
        }

        // update job counts
        groups[c_level].job_count += 1;
        groups[c_level].total_p += job.p;

    }

    // local realloc if no servers available
    if (forced_local_realloc && c_level != last_level_w_servers) {
        max_update += 1;
        partial_realloc(last_level_w_servers);
    } else {
        // add to hisotry if not done in partial_realloc
        get_job_group_server_count(job, history);
    }
}

void RCGREEDY::delete_job(RCGREEDY_Job &job, bool forced_local_realloc){
    if (job_group_assignments.find(job) == job_group_assignments.end()) {
        std::cerr << "Error deleting job " << job.id << ". Job doesn't exist." << std::endl;
        return; 
    }

    std::string group = job_group_assignments[job];
    job_group_assignments.erase(job); // erase the mapping

    // if erase failed, element doesn't exist here
    if (!id_to_jobs[group].erase(job)) {
        std::cerr << "Error deleting job " << job.id << ". Job not found in group." << std::endl;
        return;
    }

    history.clear(); // new action, remake history vector
    std::string c_level;
    std::string lowest_job_level;     // where to realloc servers to if needed
    size_t realloc_server_count = 0;


    // only perform local realloc if there are no more jobs at the level
    forced_local_realloc &= (groups[group].job_count == 1);
    if (forced_local_realloc) {
        realloc_server_count = groups[group].allocated_servers; // these are removed from group below
    }

    // move up the allocation 
    for (size_t len = group.size(); len >= 0; --len) {
        c_level = group.substr(0, len);

        // edit group information
        groups[c_level].job_count -= 1;
        groups[c_level].total_p -= job.p;

        if (forced_local_realloc && lowest_job_level.empty()) {
            // see if this is level for realloc
            if (groups[c_level].job_count) {
                lowest_job_level = (group[c_level.size()] == '1') ? c_level + "0" : c_level + "1";
            } else {
                // remove servers from level for realloc
                groups[c_level].allocated_servers -= realloc_server_count;
            }
            
        }
    }

    if (forced_local_realloc) {
        groups[lowest_job_level].allocated_servers += realloc_server_count;
        max_update += 1;
        partial_realloc(lowest_job_level);
    } else {
        // add history of local realloc isn't performed
        get_job_group_server_count(job, history);
    }
}

double RCGREEDY::get_server_count(RCGREEDY_Job &job) {
    if (job_group_assignments.find(job) == job_group_assignments.end()) {
        std::cerr << "Error finding job " << job.id << ". Job doesn't exist." << std::endl;
        return;
    }

    std::string group = job_group_assignments[job];

    if (groups[group].job_count == 1) return static_cast<double>(groups[group].allocated_servers);

    // if partial servers, this is trivial
    if (partial_servers) return static_cast<double>(groups[group].allocated_servers) / groups[group].job_count;
        
    size_t base = groups[group].allocated_servers / groups[group].job_count;
    size_t remainder = groups[group].allocated_servers % groups[group].job_count;

    if (remainder == 0) return base;

    // if it is one of the first jobs, it gets the remainder servers, otherwise it does not
    for (const RCGREEDY_Job& grouped_job : id_to_jobs[group]) {
        if (grouped_job == job) return base + 1;
        remainder -= 1;
        if (remainder == 0) return base;
    }
}

void RCGREEDY::get_job_group_server_count(RCGREEDY_Job &job, std::vector<std::pair<size_t, double>> &input) {
    if (job_group_assignments.find(job) == job_group_assignments.end()) {
        std::cerr << "Error finding job " << job.id << ". Job doesn't exist." << std::endl;
        return; 
    }

    const std::string group = job_group_assignments[job];

    if (groups[group].job_count == 1) {
        input.push_back({job.id, static_cast<double>(groups[group].allocated_servers)});
        return;
    }
    return get_group_server_count(group, input);
}

void RCGREEDY::get_all_server_count(std::vector<std::pair<size_t, double>> &input) {
    // iterate through only the lowest level ids
    for (const auto& pair : id_to_jobs) {
        get_group_server_count(pair.first, input);
    }

    return;
}


void RCGREEDY::get_group_server_count(const std::string &group, std::vector<std::pair<size_t, double>> &input) {
    

    double server_alloc = 0.0;
    size_t remainder = 0;
    bool flip = true;

    // if partial servers, this is trivial
    if (partial_servers){ 
        server_alloc = static_cast<double>(groups[group].allocated_servers) / groups[group].job_count;
    } else {
        server_alloc = static_cast<double>(groups[group].allocated_servers / groups[group].job_count) + 1;
        remainder = groups[group].allocated_servers % groups[group].job_count;
        flip = false;
    }
    

    // if it is one of the first jobs, it gets the remainder servers, otherwise it does not
    for (const RCGREEDY_Job& grouped_job : id_to_jobs[group]) {
        if (remainder > 0) {
            remainder -= 1;
        } else if (!flip) {
            server_alloc -= 1;
            flip = true;
        }
        input.push_back({grouped_job.id, server_alloc});
    }

    return;
}

std::vector<std::pair<size_t, double>> RCGREEDY::get_server_changes(RCGREEDY_Job &job) {
    return history;
}

void RCGREEDY::initalize_groups(){
    groups[""] = Group{0, server_count, 0, 0.0};
    history;
    generate_mappings(0.0, 1.0, "", current_depth);
}

void RCGREEDY::generate_mappings(double c_p_min, double c_p_max, std::string c_string, int remaining_depth){
    double diff = (c_p_max - c_p_min) / 2;

    // add new mappings
    groups[c_string + "1"] = Group{0, 0, 0, 0.0};
    groups[c_string + "0"] = Group{0, 0, 0, 0.0};

    // add new mappings for longer strings
    if (remaining_depth > 1) {
        generate_mappings(c_p_min + diff, c_p_max, c_string + "1", remaining_depth - 1);
        generate_mappings(c_p_min, c_p_max - diff, c_string + "0", remaining_depth - 1);
    } else {
        // initalize empty unordered sets to store values
        id_to_jobs[c_string + "1"];
        id_to_jobs[c_string + "0"];
    }

}

std::string RCGREEDY::get_group_id(RCGREEDY_Job &job){
    if (job_group_assignments.find(job) != job_group_assignments.end()) {
        return job_group_assignments[job];
    }
    double p_min = 0.0;
    double p_max = 1.0;
    double diff = .5;
    std::string output = "";

    for (int i = 0; i < current_depth; ++i) {
        if (job.p >= p_min + diff) {
            p_min += diff;
            output += "1";
        } else {
            p_max -= diff;
            output += "0";
        }

        diff /= 2;
    }

    // update assignments to avoid recomputation
    job_group_assignments[job] = output;
    return output;
}

void RCGREEDY::partial_realloc(const std::string &group){
    size_t depth = group.size();

    // update_count increase for group
    groups[group].update_count = max_update;

    // if at lowest point, reallocation was succesful and thus return
    if (depth == current_depth) {
        get_group_server_count(group, history); // add updates to history
        return;
    }

    std::string group0 = group + "0";
    std::string group1 = group + "1";

    // if one group has no jobs, assign all jobs to the other group
    if (!groups[group0].job_count) {
        groups[group1].allocated_servers = groups[group].allocated_servers;
        groups[group0].allocated_servers = 0;
        groups[group0].update_count = max_update;
        return partial_realloc(group1);
    } else if (!groups[group1].job_count) {
        groups[group0].allocated_servers = groups[group].allocated_servers;
        groups[group1].allocated_servers = 0;
        groups[group1].update_count = max_update;
        return partial_realloc(group0);
    }

    size_t a1 = optimal_server_count(groups[group0].total_p / groups[group0].job_count,
                                     groups[group0].job_count, 
                                     groups[group1].total_p / groups[group1].job_count,
                                     groups[group1].job_count,
                                     groups[group].allocated_servers);
    
    groups[group0].allocated_servers = a1;
    groups[group1].allocated_servers = groups[group].allocated_servers - a1;
    
    partial_realloc(group0);
    partial_realloc(group1);

    return;
} 

