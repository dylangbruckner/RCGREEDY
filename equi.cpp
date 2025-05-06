#include "equi.hpp"

EQUI::EQUI(size_t servers) : server_count(servers) {}

bool EQUI::job_exists(size_t job_id) {
    return job_map.find(job_id) != job_map.end();
}

void EQUI::insert_job(size_t job_id) {
    if (!job_exists(job_id)) {
        Scheduled_Job j{job_id, insertion_count++};
        jobs.insert(j);
        job_map[job_id] = j;
    }
}

void EQUI::delete_job(size_t job_id) {
    auto it = job_map.find(job_id);
    if (it != job_map.end()) {
        jobs.erase(it->second);
        job_map.erase(it);
    }
}

size_t EQUI::get_allocation(size_t job_id) const{
    auto it = job_map.find(job_id);
    if (it == job_map.end()) return 0;
    
    const size_t total_jobs = jobs.size();
    if (total_jobs == 0) return 0;
    
    const size_t rank = jobs.order_of_key(it->second);
    const size_t base = server_count / total_jobs;
    const size_t remainder = server_count % total_jobs;
    
    return (rank < remainder) ? base + 1 : base;
}

size_t EQUI::get_server_count() const { return server_count; }
size_t EQUI::get_current_job_count() const { return jobs.size(); }
