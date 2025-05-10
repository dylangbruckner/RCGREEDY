#ifndef EXPERIMENTS_HPP
#define EXPERIMENTS_HPP

#include "rcgreedy_base.hpp"
#include "event_generator.hpp"
#include "equi.hpp"
#include <chrono>
#include <vector>
#include <utility> // for pair
#include <memory>
#include <fstream>
#include <iomanip>
#include <algorithm>

struct SimulationResults {
    long double avg_processing_time;
    long double avg_real_time;
};


struct JobState {
    double remaining_size;
    double current_speedup;
    long double last_update_time;
    long double expected_completion;
};

void write_csv_header(const std::string& filename);
void write_csv_row(const std::string& filename, const std::string& scheduler, 
                   const std::string& param, long double value, const SimulationResults& results);
void run_experiment_option(int option, int trials, const std::string& csv_file, int options_to_run);


void experiments(size_t trials, int option, std::string csv_output_file, bool generate_graphs);


                        // going to need as input flags:
                        //      partial vs full_server
                        //      num trials
                        //      input jobs 300 default
                        //      job size lambda
                        //      queueu lambda
                        //      different num servers
                        //      flag to deterime which options to run
                        //      seperate expirements for different strategies of reallocation with just rcgreedy
                        //      seperate expirements to see how rcgreedy's computation changes over time

const int E = 1;
const int R1 = 2;
const int R2 = 4;     // r=2 (2^2)
const int R3 = 8;     // r=3 (2^3)
const int R4 = 16;    // r=4 (2^4)
const int R5 = 32;    // r=5 (2^5)
const int R6 = 64;    // r=6 (2^6)
const int R7 = 128;   // r=7 (2^7)
const int R8 = 256;   // r=8 (2^8)
const int R9 = 512;


std::vector<SimulationResults> experiments_new(int options_to_run, size_t num_servers = 1000, double job_spacing_lambda = 1.0, 
                     double job_size_lambda = 9.0, bool partial_servers = true, 
                     size_t jobs = 300, size_t full_realloc_count = 1);


SimulationResults simulation_runner(
    boost::heap::priority_queue<Event, boost::heap::compare<Compare_Event>>& events,
    int scheduler_type,
    size_t num_servers,
    bool partial_servers,
    int r_depth = 0, 
    size_t full_realloc_count = 10,
    double job_size_lambda = 1.0
);


#endif // EXPERIMENTS_HPP