#include "event_generator.hpp"

boost::heap::priority_queue<Event, boost::heap::compare<Compare_Event>> generate_events(
    size_t num_events, double arrival_lambda, double job_size_lambda) {

    // set up distributions
    std::random_device rd;
    std::mt19937 generator(rd());
    std::exponential_distribution<long double> arrival(arrival_lambda);
    std::exponential_distribution<double> job_size(job_size_lambda);
    std::uniform_real_distribution<double> speedup(0.0, 1.0);

    // tracks current time
    long double elapsed_time = 0.0;

    boost::heap::priority_queue<Event, boost::heap::compare<Compare_Event>> pq;

    for (size_t i = 0; i < num_events; ++i) {

        // generate space between events and job size, add to pq
        elapsed_time += arrival(generator);
        pq.push(Event{ARRIVAL, elapsed_time, Job{i, 0.0, job_size(generator), 0.0, -1, speedup(generator)}});
    }

    return pq;
}