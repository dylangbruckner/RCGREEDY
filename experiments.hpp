#ifndef EXPERIMENTS_HPP
#define EXPERIMENTS_HPP

#include "rcgreedy_base.hpp"
#include "event_generator.hpp"
#include "equi.hpp"

void experiments(size_t server_count, size_t trials, double job_size_lambda, double arrival_lambda,
                        int data_tracking_flag, std::string csv_output_file, bool generate_graphs);


#endif // EXPERIMENTS_HPP