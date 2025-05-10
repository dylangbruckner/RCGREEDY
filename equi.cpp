#include "equi.hpp"

EQUI::EQUI(size_t servers, bool partial_server_allocs = false) : server_count(servers), partial_servers(partial_server_allocs) {}

bool EQUI::job_exists(size_t job_id) {
    return jobs.find(job_id) != jobs.end();
}

void EQUI::insert_job(size_t job_id) {
    if (!job_exists(job_id)) {
        jobs.insert(job_id);
    } else {
        std::cerr << "Error adding job " << job_id << ". Job already exists" << std::endl;
    }
}

void EQUI::delete_job(size_t job_id) {
    if (job_exists(job_id)) {
        jobs.erase(job_id);
    } else {
        std::cerr << "Error deleting job " << job_id << ". Job doesn't exist" << std::endl;
    }
}

double EQUI::get_allocation(size_t job_id) const{
    if (!jobs.size()) return 0;
    
    // if partial servers, this is trivial
    if (partial_servers){ 
        return static_cast<double>(server_count) / jobs.size();
    }

    size_t server_alloc = server_count / jobs.size() + 1;
    size_t remainder = server_count % jobs.size();
    

    // if it is one of the first jobs, it gets the remainder servers, otherwise it does not
    for (const size_t job_ids : jobs) {
        if (!remainder) return server_alloc - 1;
        if (job_ids == job_id) return server_alloc;
        remainder -= 1;
    }
    return server_alloc - 1;
}

void EQUI::get_all_allocations(std::vector<std::pair<size_t, double>> &input) {
    if (!jobs.size()) return;
    
    double server_alloc = 0.0;
    size_t remainder = 0;
    bool flip = true;

    // if partial servers, this is trivial
    if (partial_servers){ 
        server_alloc = static_cast<double>(server_count) / jobs.size();
    } else {
        server_alloc = static_cast<double>(server_count / jobs.size()) + 1;
        remainder = server_count % jobs.size();
        flip = false;
    }
    

    // if it is one of the first jobs, it gets the remainder servers, otherwise it does not
    for (const size_t job_id : jobs) {
        if (remainder > 0) {
            remainder -= 1;
        } else if (!flip) {
            server_alloc -= 1;
            flip = true;
        }
        input.push_back({job_id, server_alloc});
    }

    return;
}

size_t EQUI::get_server_count() const { return server_count; }
size_t EQUI::get_current_job_count() const { return jobs.size(); }
