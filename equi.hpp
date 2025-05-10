#ifndef EQUI_HPP
#define EQUI_HPP

#include <unordered_set>
#include <iostream>
#include <vector>


class EQUI {
    private:
        
        size_t server_count;
        std::unordered_set<size_t> jobs; 
        bool partial_servers;

    public:
        EQUI(size_t servers, bool partial_server_allocs);

        bool job_exists(size_t job_id);
        void insert_job(size_t job_id);
        void delete_job(size_t job_id);

        // gets the number of servers allocated to any given job
        double get_allocation(size_t job_id) const;
        
        // get the allocations for every job in the system, put into the input vector
        void get_all_allocations(std::vector<std::pair<size_t, double>> &input);

        size_t get_server_count() const;
        size_t get_current_job_count() const;

        /*
        * returns the speedup factor of any job with p as the speedup 
        * parameter and servers allocated servers
        */
        inline double speedup_factor(double p, double servers) {
            return 1.0 / ((p / servers) + 1 - p);
        }
    
};

#endif // EQUI_HPP