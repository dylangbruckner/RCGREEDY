#ifndef EVENT_GENERATOR_HPP
#define EVENT_GENERATOR_HPP

#include <random>
#include <boost/heap/priority_queue.hpp>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace __gnu_pbds;

// tree used as internal data structure for schedulers
template<typename T>
using ordered_set = tree<
    T,
    null_type,
    std::less<T>,
    rb_tree_tag,
    tree_order_statistics_node_update>;


// jobs
struct Job {
    size_t job_id;           // used by the scheduler to differentiate jobs
    long double arrival_time;
    double size;             // exp distributed
    double remaining_size;   // hidden from scheduler, used for simulation
    int current_group;       // the server group this job is assigned to
    double p;                // speedup paramater
};

// events
struct Event {
    int event_type;          // as below
    long double event_time;  // when the event will occur (used as a pq key)
    Job job;
};

// used as a comparison function in pq
struct Compare_Event {
    bool operator()(const Event& e1, const Event& e2) const {
        return e1.event_time > e2.event_time; 
    }
};

// event types
const int ARRIVAL = 0;       // new arrival into the scheduler
const int COMPLETION = 1;    // event completion


/* 
*   returns a priority queue containing jobs generated with a job_size_lambda exponential distribution and 
*   spaced according to a poisson process with arrival_lambda
*/
boost::heap::priority_queue<Event, boost::heap::compare<Compare_Event>> generate_events(
    size_t num_events, double arrival_lambda, double job_size_lambda);




#endif // EVENT_GENERATOR_HPP