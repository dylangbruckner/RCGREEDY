#include "rcgreedy_base.hpp"


RCGREEDY::RCGREEDY(size_t servers, int max_depth, double lambda) : 
    server_count(servers), current_depth(std::max(0, std::min(max_depth, MAX_DEPTH))), job_size_lambda(lambda) {
    initalize_groups();

    // initally, give all of servers to the top group
    groups[""].allocated_servers = server_count;
}

void RCGREEDY::add_job(RCGREEDY_Job &job, bool forced_local_realloc) {
    if (current_jobs.find(job) != current_jobs.end()) {
        return; // todo some error here
    }

    // add tracking data
    job.insert_order = input_count;
    input_count += 1;
    current_jobs.insert(job);

    std::string group = get_group_id(job);
    std::string last_level_w_servers = ""; // used for local realloc
    std::string c_level;
    size_t current_update = groups[""].update_count;

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
    }
    
    // todo could also return sever allocation here
}

void RCGREEDY::delete_job(RCGREEDY_Job &job, bool forced_local_realloc){
    if (current_jobs.find(job) == current_jobs.end()) {
        return; // todo some error here
    }

    current_jobs.erase(job);

    std::string group = get_group_id(job);
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
            // remove servers from level for realloc
            groups[c_level].allocated_servers -= realloc_server_count;

            // see if this is level for realloc
            if (groups[c_level].job_count) {
                lowest_job_level = (group[c_level.size()] == '1') ? c_level + "0" : c_level + "1";
            }
        }
    }

    if (forced_local_realloc) {
        groups[lowest_job_level].allocated_servers += realloc_server_count;
        max_update += 1;
        partial_realloc(lowest_job_level);
    } 
}

void RCGREEDY::initalize_groups(){
    groups[""] = Group{0, 0, 0, 0.0, 0.0, 1.0};
    generate_mappings(0.0, 1.0, "", current_depth);
}

void RCGREEDY::generate_mappings(double c_p_min, double c_p_max, std::string c_string, int remaining_depth){
    double diff = (c_p_max - c_p_min) / 2;

    // add new mappings
    groups[c_string + "1"] = Group{0, 0, 0, 0.0, c_p_min + diff, c_p_max};
    groups[c_string + "0"] = Group{0, 0, 0, 0.0, c_p_min, c_p_max - diff};

    // add new mappings for longer strings
    if (remaining_depth > 1) {
        generate_mappings(c_p_min + diff, c_p_max, c_string + "1", remaining_depth - 1);
        generate_mappings(c_p_min, c_p_max - diff, c_string + "0", remaining_depth - 1);
    }

}

std::string RCGREEDY::get_group_id(RCGREEDY_Job &job){
    if (job_group_assignments.find(job.id) != job_group_assignments.end()) {
        return job_group_assignments[job.id];
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
    job_group_assignments[job.id] = output;
    return output;
}

void RCGREEDY::partial_realloc(const std::string &group){
    size_t depth = group.size();

    // update_count increase for group
    groups[group].update_count = max_update;

    // if at lowest point, reallocation was succesful and thus return
    if (depth == current_depth) {
        return;
    }

    std::string group1 = group + "0";
    std::string group2 = group + "1";

    // if one group has no jobs, assign all jobs to the other group
    if (!groups[group1].job_count) {
        groups[group2].allocated_servers = groups[group].allocated_servers;
        return partial_realloc(group1);
    } else if (!groups[group + "1"].job_count) {
        groups[group2].allocated_servers = groups[group].allocated_servers;
        return partial_realloc(group2);
    }

    size_t a1 = optimal_server_count(groups[group1].total_p / groups[group1].job_count,
                                     groups[group1].job_count, 
                                     groups[group2].total_p / groups[group2].job_count,
                                     groups[group2].job_count,
                                     groups[group].allocated_servers);
    
    groups[group1].allocated_servers = a1;
    groups[group2].allocated_servers = groups[group].allocated_servers - a1;
    
    partial_realloc(group1);
    partial_realloc(group2);

    return;
} 

