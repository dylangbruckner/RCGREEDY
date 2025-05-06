#ifndef EQUI_HPP
#define EQUI_HPP

#include "event_generator.hpp"


class EQUI {
    private:
        struct Scheduled_Job {
            size_t id;
            size_t insert_order;

            bool operator<(const Scheduled_Job& other) const {
                return insert_order < other.insert_order;
            }
        };

        ordered_set<Scheduled_Job> jobs;
        std::map<size_t, Scheduled_Job> job_map;
        size_t server_count;
        size_t insertion_count = 0;

    public:
        EQUI(size_t servers);

        bool job_exists(size_t job_id);
        void insert_job(size_t job_id);
        void delete_job(size_t job_id);

        // gets the number of servers allocated to any given job
        size_t get_allocation(size_t job_id) const;

        size_t get_server_count() const;
        size_t get_current_job_count() const;
};

#endif // EQUI_HPP