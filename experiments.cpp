#include "experiments.hpp"

void write_csv_header(const std::string& filename) {
    std::ofstream file(filename, std::ios::trunc);
    if(file.is_open()) {
        file << "Scheduler,Parameter,Value,AverageProcessingTime,AvgRealTime\n";
    }
}

// Update CSV row to include scheduler type and composite time
void write_csv_row(const std::string& filename, const std::string& scheduler, 
                   const std::string& param, long double value, const SimulationResults& results) {
    std::ofstream file(filename, std::ios::app);
    if(file.is_open()) {
        file << scheduler << "," << param << "," << value << ","
             << std::fixed << std::setprecision(7) 
             << results.avg_processing_time << ","
             << results.avg_real_time << "\n";
    }
}

// Helper to get scheduler name from flag
std::string get_scheduler_name(int flag) {
    switch(flag) {
        case E: return "EQUI";
        case R1: return "R1";
        case R2: return "R2";
        case R3: return "R3";
        case R4: return "R4";
        case R5: return "R5";
        case R6: return "R6";
        case R7: return "R7";
        case R8: return "R8";
        case R9: return "R9";
        default: return "Unknown";
    }
}

void run_experiment_option(int option, int trials, const std::string& csv_file, int options_to_run) {
    // Extract enabled schedulers
    std::vector<int> enabled_schedulers;
    if(options_to_run & E) enabled_schedulers.push_back(E);
    const std::vector<std::pair<int, int>> r_flags = {{R1,1},{R2,2},{R3,3},{R4,4},
                                                     {R5,5},{R6,6},{R7,7},{R8,8}};

    for(const auto& [flag, _] : r_flags) {
        if(options_to_run & flag) enabled_schedulers.push_back(flag);
    }

    switch(option) {
        case 1: { // Vary num servers
            for(size_t servers = 50; servers <= 200; servers += 25) {
                // Store results per scheduler
                std::unordered_map<int, SimulationResults> total_results;

                for(int t = 0; t < trials; t++) {
                    auto results = experiments_new(options_to_run, servers);
                    
                    // Assume results vector order matches enabled_schedulers order
                    for(size_t i = 0; i < results.size(); i++) {
                        int scheduler_flag = enabled_schedulers[i];
                        total_results[scheduler_flag].avg_processing_time += results[i].avg_processing_time;
                        total_results[scheduler_flag].avg_real_time += results[i].avg_real_time;
                    }
                }

                // Write averages per scheduler
                for(const auto& [flag, total] : total_results) {
                    SimulationResults avg{
                        total.avg_processing_time/trials,
                        total.avg_real_time/trials
                    };
                    write_csv_row(csv_file, get_scheduler_name(flag), 
                                "Servers", servers, avg);
                }
            }
            break;
        }
        
        case 2: { // Vary job size lambda
            for(double lambda = 0.1; lambda <= 20; lambda += 0.5) {
                 // Store results per scheduler
                std::unordered_map<int, SimulationResults> total_results;

                for(int t = 0; t < trials; t++) {
                    auto results = experiments_new(options_to_run, 1000, 20.0, lambda, false);
                     // Assume results vector order matches enabled_schedulers order
                    for(size_t i = 0; i < results.size(); i++) {
                        int scheduler_flag = enabled_schedulers[i];
                        total_results[scheduler_flag].avg_processing_time += results[i].avg_processing_time;
                        total_results[scheduler_flag].avg_real_time += results[i].avg_real_time;
                    }
                }

                 // Write averages per scheduler
                for(const auto& [flag, total] : total_results) {
                    SimulationResults avg{
                        total.avg_processing_time/trials,
                        total.avg_real_time/trials
                    };
                    write_csv_row(csv_file, get_scheduler_name(flag), 
                                "JobSizeLambda", lambda, avg);
                }
            }
            break;
        }

        case 3: { // Vary arrival lambda
            for(double lambda = 0.5; lambda <= 2.5; lambda += 0.5) {
                // Store results per scheduler
                std::unordered_map<int, SimulationResults> total_results;

                for(int t = 0; t < trials; t++) {
                    auto results = experiments_new(options_to_run, 100, lambda, 1.0);
                    // Assume results vector order matches enabled_schedulers order
                    for(size_t i = 0; i < results.size(); i++) {
                        int scheduler_flag = enabled_schedulers[i];
                        total_results[scheduler_flag].avg_processing_time += results[i].avg_processing_time;
                        total_results[scheduler_flag].avg_real_time += results[i].avg_real_time;
                    }
                }
                // Write averages per scheduler
                for(const auto& [flag, total] : total_results) {
                    SimulationResults avg{
                        total.avg_processing_time/trials,
                        total.avg_real_time/trials
                    };
                write_csv_row(csv_file, get_scheduler_name(flag), 
                                "JobSpacingLambda", lambda, avg);
                }
                
            }
            break;
        }

        case 4: { // Partial vs full servers
            for(bool partial : {true, false}) {
                // Store results per scheduler
                std::unordered_map<int, SimulationResults> total_results;

                for(int t = 0; t < trials; t++) {
                    auto results = experiments_new(options_to_run, 100, 1.0, 1.0, partial);
                    // Assume results vector order matches enabled_schedulers order
                    for(size_t i = 0; i < results.size(); i++) {
                        int scheduler_flag = enabled_schedulers[i];
                        total_results[scheduler_flag].avg_processing_time += results[i].avg_processing_time;
                        total_results[scheduler_flag].avg_real_time += results[i].avg_real_time;
                    }
                }
                 // Write averages per scheduler
                for(const auto& [flag, total] : total_results) {
                    SimulationResults avg{
                        total.avg_processing_time/trials,
                        total.avg_real_time/trials
                    };
                write_csv_row(csv_file, get_scheduler_name(flag), 
                                "PartialServers", partial, avg);
                }
            }
            break;
        }

        case 5: { // Reallocation frequency
            for(size_t freq : {1, 5, 10, 15, 20}) {
               // Store results per scheduler
                std::unordered_map<int, SimulationResults> total_results;
                for(int t = 0; t < trials; t++) {
                    auto results = experiments_new(options_to_run, 100, 1.0, 1.0, true, 1000, freq);
                    // Assume results vector order matches enabled_schedulers order
                    for(size_t i = 0; i < results.size(); i++) {
                        int scheduler_flag = enabled_schedulers[i];
                        total_results[scheduler_flag].avg_processing_time += results[i].avg_processing_time;
                        total_results[scheduler_flag].avg_real_time += results[i].avg_real_time;
                    }
                }
                 // Write averages per scheduler
                for(const auto& [flag, total] : total_results) {
                    SimulationResults avg{
                        total.avg_processing_time/trials,
                        total.avg_real_time/trials
                    };
                write_csv_row(csv_file, get_scheduler_name(flag), 
                                "ReallocationFrequency", freq, avg);
                }
            }
            break;
        }
    }
}

void experiments(size_t trials, int option, std::string csv_output_file, bool generate_graphs) {
    write_csv_header(csv_output_file);
    run_experiment_option(option, trials, csv_output_file, E|R1|R3|R4|R5|R7|R8);
    
    if(generate_graphs) {
        // Add Python plotting code here
    }
}

std::vector<SimulationResults> experiments_new(int options_to_run, 
                                              size_t num_servers, 
                                              double job_spacing_lambda, 
                                              double job_size_lambda, 
                                              bool partial_servers, 
                                              size_t jobs, 
                                              size_t full_realloc_count) {

    // Generate the base event queue
    auto base_events = generate_events(jobs, job_spacing_lambda, job_size_lambda);

    // Store results [EQUI, R1, R2, ..., R8]
    std::vector<SimulationResults> results;

    // Run EQUI if selected
    if (options_to_run & E) {
        auto res = simulation_runner(base_events, E, num_servers, partial_servers);
        results.push_back(res);
    }

   // Run RCGREEDY variants
    const std::vector<std::pair<int, int>> r_flags = {
        {R1, 1}, {R2, 2}, {R3, 3}, {R4, 4},
        {R5, 5}, {R6, 6}, {R7, 7}, {R8, 8}, 
        {R9, 9}
    };

    for(const auto& [flag, depth] : r_flags) {
        if(options_to_run & flag) {
            auto res = simulation_runner(base_events, flag, num_servers, partial_servers, depth, full_realloc_count, job_size_lambda);
            results.push_back(res);
        }
    }

    return results;

}


SimulationResults simulation_runner(
    boost::heap::priority_queue<Event, boost::heap::compare<Compare_Event>>& events,
    int scheduler_type,
    size_t num_servers,
    bool partial_servers,
    int r_depth,
    size_t full_realloc_count, 
    double job_size_lambda
) {
    auto event_queue = events;
    std::unordered_map<size_t, JobState> job_states;
    std::unordered_map<size_t, Job> jobs;
    std::unordered_map<size_t, long double> job_arrival_times;
    std::vector<double> processing_times;
    double total_real_time = 0.0;
    size_t realloc_counter = full_realloc_count;
    long double current_time = 0.0;

    std::unique_ptr<RCGREEDY> rcgreedy;
    std::unique_ptr<EQUI> equi;
    
    if(scheduler_type == E) {
        equi = std::make_unique<EQUI>(num_servers, partial_servers);
    } else {
        rcgreedy = std::make_unique<RCGREEDY>(num_servers, r_depth, 1.0 / job_size_lambda, partial_servers);
    }

    auto update_job_processing = [&](size_t job_id, long double update_time, double servers) {
        auto& state = job_states[job_id];
        
        // Calculate processed work since last update
        double elapsed = update_time - state.last_update_time;
        state.remaining_size -= state.current_speedup * elapsed;
        state.last_update_time = update_time;

        // Get new speedup factor
        double new_speedup;
        if(scheduler_type == E) {
            new_speedup = equi->speedup_factor(jobs[job_id].p, servers);
        } else {
            new_speedup = rcgreedy->speedup_factor(jobs[job_id].p, servers);
        }
        
        if(new_speedup < 1e-6) new_speedup = 1e-6; // Prevent division by zero
        
        // Update state and reschedule
        state.current_speedup = new_speedup;
        double new_processing = state.remaining_size / new_speedup;
        state.expected_completion = update_time + new_processing;

        // Add new completion event
        Event new_event;
        new_event.event_type = COMPLETION;
        new_event.event_time = state.expected_completion;
        new_event.job.job_id = job_id;
        new_event.job.p = jobs[job_id].p;
        event_queue.push(new_event);
    };

    auto process_allocation_changes = [&](long double current_time) {
        if(scheduler_type == E) {
            // EQUI affects all jobs
            std::vector<std::pair<size_t, double>> output;
            equi->get_all_allocations(output);
            for(auto& [job_id, servers] : output) {
                update_job_processing(job_id, current_time, servers);
            }
        } else {
            // RCGREEDY only affects changed jobs
            auto changes = rcgreedy->get_server_changes();
            for(auto& [job_id, servers] : changes) {
                if(job_states.count(job_id)) {
                    update_job_processing(job_id, current_time, servers);
                }
            }
        }
    };

    while(!event_queue.empty()) {
        auto event = event_queue.top();
        event_queue.pop();
        current_time = event.event_time;

        if(event.event_type == ARRIVAL) {
            // Track arrival time and store job info
            job_arrival_times[event.job.job_id] = current_time;
            jobs[event.job.job_id] = event.job;
            JobState state;
            state.remaining_size = event.job.size;
            state.current_speedup = 1.0; // Will be updated immediately
            state.last_update_time = current_time;
            state.expected_completion = 0.0; // Will be set soon
            job_states[event.job.job_id] = state;
            auto start = std::chrono::high_resolution_clock::now();
            
            if(scheduler_type == E) {
                equi->insert_job(event.job.job_id);
            } else {
                RCGREEDY::RCGREEDY_Job job;
                job.id = event.job.job_id;
                job.p = event.job.p;
                
                if(realloc_counter == 0) {
                    rcgreedy->full_realloc();
                    realloc_counter = full_realloc_count;
                }
                rcgreedy->add_job(job, true);
                realloc_counter--;
            }
            
            // Process allocation changes and update all affected jobs
            process_allocation_changes(current_time);

            auto end = std::chrono::high_resolution_clock::now();
            total_real_time += std::chrono::duration<double>(end - start).count();

        } else if(event.event_type == COMPLETION) {
            auto it = job_states.find(event.job.job_id);
            if(it == job_states.end()) continue;
            
            auto& state = it->second;
            if(std::abs(event.event_time - state.expected_completion) > 1e-6) {
                continue; // Skip outdated event
            }

            // Record processing time
            processing_times.push_back(current_time - job_arrival_times[event.job.job_id]);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            if(scheduler_type == E) {
                equi->delete_job(event.job.job_id);
            } else {
                RCGREEDY::RCGREEDY_Job job;
                job.id = event.job.job_id;
                job.p = event.job.p;
                
                if(realloc_counter == 0) {
                    rcgreedy->full_realloc();
                    realloc_counter = full_realloc_count;
                }
                rcgreedy->delete_job(job, true);
                realloc_counter--;
            }
            
            // Process allocation changes and update affected jobs
            process_allocation_changes(current_time);
            
            job_states.erase(it);
            jobs.erase(event.job.job_id);
            job_arrival_times.erase(event.job.job_id);

            auto end = std::chrono::high_resolution_clock::now();
            total_real_time += std::chrono::duration<double>(end - start).count();
        }
    }

    // Calculate averages
    long double avg_processing = processing_times.empty() ? 0.0 :
        std::accumulate(processing_times.begin(), processing_times.end(), 0.0) / processing_times.size();
        
    long double avg_real = processing_times.empty() ? 0.0 :
        total_real_time / processing_times.size();

    return {avg_processing, total_real_time};
}