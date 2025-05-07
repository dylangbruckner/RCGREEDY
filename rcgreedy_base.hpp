#ifndef RCGREEDY_BASE_HPP
#define RCGREEDY_BASE_HPP
#include "event_generator.hpp"
#include <cmath>
#include <unordered_map>
#include <unordered_set>

const double EPSILON = 1e-6; // used for floating point calculations

class RCGREEDY {
    static constexpr int MAX_DEPTH = 10;  // maximum recursion depth of our scheduler 

public:
    struct RCGREEDY_Job {
        size_t id;
        size_t insert_order;
        double p;

        bool operator<(const RCGREEDY_Job& other) const {
            return insert_order < other.insert_order;
        }

        bool operator=(const RCGREEDY_Job& other) const {
            return id == other.id;
        }
    };

    RCGREEDY(size_t servers, int max_depth, double lambda);

    /*
    * performa a full reallocation of the entire system, based on the RCGREEDY
    * reallocation formula
    */
    void full_realloc();

    /*
    * add job Job to scheduler. If forced_local_realloc, when the job is added
    * if their are no servers allocated to it's current group, it forcefully 
    * reallocates the closet deapth above where there are servers
    */
    void add_job(RCGREEDY_Job &Job, bool forced_local_realloc);
    /*
    * delete job Job to scheduler. If forced_local_realloc and the job is the last job 
    * in the group, the servers will be reallocated to the closet deapth above where
    * there are jobs
    */
    void delete_job(RCGREEDY_Job &Job, bool forced_local_realloc);


    /*
    * returns the speedup factor of any job with p as the speedup 
    * parameter and servers allocated servers
    */
    inline double speedup_factor(double p, double servers) {
        return 1.0 / ((p / servers) + 1 - p);
    }
    

private:

    // groups of servers, lazyily updated
    struct Group {
        size_t job_count = 0;                   // total jobs in this group
        size_t allocated_servers = 0;           // number of servers available in group
        size_t update_count = 0;                // if server count is old, this will be less than parent groups   
        double total_p = 0.0;                   // total p-value of all jobs within group
        double p_min, p_max;                    // range of group (p_min, p_max] (last group is (p_min, 1))
    };

    std::unordered_map<std::string, Group> groups;                 // mapping of group id strings to their groups
    std::unordered_map<size_t, std::string> job_group_assignments; // maps jobs to their group id
    std::unordered_set<RCGREEDY_Job> current_jobs;                 // contains all jobs currently in the scheduler
    size_t server_count;
    int current_depth;
    size_t max_update;      // todo delete this probably
    double job_size_lambda; // lambda in exponential distriubtion of job size
    size_t input_count = 0; // unique ordering id for inputs

    // initalizes all mappings of strings to groups
    void initalize_groups();

    // recursive helper for initialize groups
    void generate_mappings(double c_p_min, double c_p_max, std::string c_string, int remaining_depth);

    // gets smallest group id for any given job
    std::string get_group_id(RCGREEDY_Job &job);

    // reallocate from group downwards
    void partial_realloc(const std::string &group); 

    // same thing as above but with parallelization
    void partial_realloc_paralellization(const std::string &group); // todo

    // returns the optimal number of servers to allocate to the less parallelizable class
    // p1 is the less parallelizable class
    inline size_t optimal_server_count(double p1, size_t jobs_count_1, double p2, size_t jobs_count_2, size_t total_servers) {
        size_t a1 = 0;
        double max_value = 0.0;
        double current_value;
        double const_1 = jobs_count_1 * job_size_lambda;
        double const_2 = jobs_count_2 * job_size_lambda;

        for (size_t temp_a1 = 0; temp_a1 <= total_servers; ++temp_a1) {
            current_value = const_1 * speedup_factor(p1, temp_a1 / jobs_count_1) + const_2 * speedup_factor(p2, (total_servers - temp_a1) / jobs_count_2);

            // if current value is greater than or equal to max value, take higher a1 value
            if (max_value - current_value < EPSILON) {
                a1 = temp_a1;
                max_value = current_value;
            }
        }

        return a1;
    }

};




#endif // RCGREEDY_BASE_HPP