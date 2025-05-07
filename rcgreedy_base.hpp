#ifndef RCGREEDY_BASE_HPP
#define RCGREEDY_BASE_HPP
#include "event_generator.hpp"
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include <iostream>

const double EPSILON = 1e-6; // used for floating point calculations
class RCGREEDY {
    static constexpr int MAX_DEPTH = 10;  // maximum recursion depth of our scheduler 

public:
    struct RCGREEDY_Job {
        size_t id = 0;
        double p = 0.0;

        RCGREEDY_Job() = default;

        bool operator<(const RCGREEDY_Job& other) const {
            return id < other.id;
        }

        bool operator==(const RCGREEDY_Job& other) const {
            return id == other.id;
        }
    };

    RCGREEDY(size_t servers, int max_depth, double average_size, bool partial_server_allocs = false);

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
    * returns the server count allocated to any job. 
    * For large scale server allocation numbers, get_server_changes, 
    * get job_group_server_count, and get_all_server_count are more 
    * efficient
    */
    double get_server_count(RCGREEDY_Job &job);

    /*
    * adds the server count allocated to any job group to the given vector, as a vector of 
    * pairs of job id's and allocated servers. If partial allocation is on,
    * all of the allocations will be equal.
    */
    void get_job_group_server_count(RCGREEDY_Job &job, std::vector<std::pair<size_t, double>> &input);
    
     /*
    * gets the server count of every job and adds it to the vector input
    */
    void get_all_server_count(std::vector<std::pair<size_t, double>> &input);
    
    // todo maybe add an option to turn this off
    /*
    * returns a vector of any jobs (as ids) and their allocations (as doubles) that have changed
    * in the last insertion/deletion or server update. 
    */
    std::vector<std::pair<size_t, double>> get_server_changes(RCGREEDY_Job &job);
    /*
    * returns the speedup factor of any job with p as the speedup 
    * parameter and servers allocated servers
    */
    inline double speedup_factor(double p, double servers) {
        return 1.0 / ((p / servers) + 1 - p);
    }
    

private:


    // 'custom' hash function for mapping RCGREEDY_Jobs
    struct Job_Hash {
        size_t operator()(const RCGREEDY_Job& job) const {
            return std::hash<size_t>{}(job.id);
        }
    };

    // groups of servers, lazyily updated
    struct Group {
        size_t job_count = 0;                   // total jobs in this group
        size_t allocated_servers = 0;           // number of servers available in group
        size_t update_count = 0;                // if server count is old, this will be less than parent groups   
        double total_p = 0.0;                   // total p-value of all jobs within group
    };

    std::unordered_map<std::string, Group> groups;                         // mapping of group id strings to their groups
    std::unordered_map<std::string, std::unordered_set<RCGREEDY_Job, Job_Hash>> id_to_jobs; // mapping of group id strings to sorted vector of jobs in group, only for lowest group
    std::unordered_map<RCGREEDY_Job, std::string, Job_Hash> job_group_assignments;   // maps jobs to their group id

    size_t server_count;
    int current_depth;
    size_t max_update = 0;            
    double maximization_constant; // see GREEDY* optimization formula. 1/E(X), where X is the job size distribution

    bool partial_servers;         // if true, jobs can utilize portions of servers; otherwise, they need a whole number of servers to operate

    std::vector<std::pair<size_t, double>> history; // vector containing recent (last insert/delete changes) server allocations

    // initalizes all mappings of strings to groups
    void initalize_groups();

    // recursive helper for initialize groups
    void generate_mappings(double c_p_min, double c_p_max, std::string c_string, int remaining_depth);

    // gets the server count for all elements in group
    void get_group_server_count(const std::string &group, std::vector<std::pair<size_t, double>> &input);

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
        double const_1 = jobs_count_1 * maximization_constant;
        double const_2 = jobs_count_2 * maximization_constant;

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