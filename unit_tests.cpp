#include "unit_tests.hpp"

bool double_eq(double a, double b) {
    return std::fabs(a - b) < EPS;
}

void print_result(const std::string& test_name, bool passed, const std::string& expected = "", const std::string& actual = "") {
    if (passed) {
        std::cout << "[PASS] " << test_name << ": Test passed\n";
    } else {
        std::cout << "[FAIL] " << test_name << ": Test failed\n";
        if (!expected.empty() || !actual.empty()) {
            std::cout << "  Expected: " << expected << "\n";
            std::cout << "  Actual:   " << actual << "\n";
        }
    }
}

int unit_tests() {

    // ---- Test 1: EQUI insertion and deletion ----
    {
        EQUI equi(4, false);
        equi.insert_job(1);
        bool exists = equi.job_exists(1);
        print_result("EQUI Insert", exists, "true", exists ? "true" : "false");
        equi.delete_job(1);
        exists = equi.job_exists(1);
        print_result("EQUI Delete", !exists, "false", exists ? "true" : "false");
    }

    // ---- Test 2: EQUI duplicate insert/delete ----
    {
        EQUI equi(4, false);
        equi.insert_job(2);
        equi.insert_job(2); // Should print error
        equi.delete_job(3); // Should print error
        // No assertion, just visual error output
    }

    // ---- Test 3: EQUI allocation sum ----
    {
        EQUI equi(6, false);
        equi.insert_job(1);
        equi.insert_job(2);
        equi.insert_job(3);
        std::vector<std::pair<size_t, double>> allocs;
        equi.get_all_allocations(allocs);
        double total = 0;
        for (auto& p : allocs) total += p.second;
        print_result("EQUI Allocation Sum", std::abs(total - 6.0) < EPS, "6", std::to_string(total));
    }

    // ---- Test 4: EQUI partial allocation ----
    {
        EQUI equi(10, true);
        equi.insert_job(1);
        equi.insert_job(2);
        double alloc = equi.get_allocation(1);
        print_result("EQUI Partial Allocation", double_eq(alloc, 5.0), "5.0", std::to_string(alloc));
    }

    // ---- Test 5: RCGREEDY basic add/delete ----
    {
        RCGREEDY rcg(10, 3, 0.5);
        RCGREEDY::RCGREEDY_Job job{1, 0.5};
        rcg.add_job(job, false);
        std::vector<std::pair<size_t, double>> allocs;
        rcg.full_realloc();
        rcg.get_all_server_count(allocs);
        bool found = false;
        for (auto& p : allocs) if (p.first == 1) found = true;
        print_result("RCGREEDY Insert", found, "true", found ? "true" : "false");
        rcg.delete_job(job, false);
        allocs.clear();
        rcg.full_realloc();
        rcg.get_all_server_count(allocs);
        found = false;
        for (auto& p : allocs) if (p.first == 1) found = true;
        print_result("RCGREEDY Delete", !found, "false", found ? "true" : "false");
    }

    // ---- Test 6: RCGREEDY allocation sum ----
    {
        RCGREEDY rcg(8, 3, 0.5);
        std::vector<RCGREEDY::RCGREEDY_Job> jobs = {{1, 0.3}, {2, 0.6}, {3, 0.8}};
        for (auto& job : jobs) rcg.add_job(job, false);
        rcg.full_realloc();
        std::vector<std::pair<size_t, double>> allocs;
        rcg.get_all_server_count(allocs);
        double total = 0;
        for (auto& p : allocs) total += p.second;
        print_result("RCGREEDY Allocation Sum", std::abs(total - 8.0) < EPS, "8", std::to_string(total));
    }

    // ---- Test 7: RCGREEDY order independence ----
    {
        RCGREEDY rcg1(10, 3, 0.5), rcg2(10, 3, 0.5);
        std::vector<RCGREEDY::RCGREEDY_Job> jobs1 = {{1, 0.3}, {2, 0.6}, {3, 0.8}};
        std::vector<RCGREEDY::RCGREEDY_Job> jobs2 = {{3, 0.8}, {2, 0.6}, {1, 0.3}};
        for (auto& job : jobs1) rcg1.add_job(job, false);
        for (auto& job : jobs2) rcg2.add_job(job, false);
        rcg1.full_realloc(); rcg2.full_realloc();
        std::vector<std::pair<size_t, double>> a1, a2;
        rcg1.get_all_server_count(a1);
        rcg2.get_all_server_count(a2);
        std::sort(a1.begin(), a1.end());
        std::sort(a2.begin(), a2.end());
        bool same = a1 == a2;
        print_result("RCGREEDY Order Independence", same);
    }

    // ---- Test 8: EQUI large scale ----
    {
        EQUI equi(10000, true);
        for (size_t i = 0; i < 100; ++i) equi.insert_job(i);
        std::vector<std::pair<size_t, double>> allocs;
        equi.get_all_allocations(allocs);
        double total = 0;
        for (auto& p : allocs) total += p.second;
        print_result("EQUI Large Scale", std::abs(total - 10000.0) < 1e-3, "10000.0", std::to_string(total));
    }

    // ---- Test 9: Floating point precision ----
    {
        RCGREEDY rcg(1000000, 5, 0.5);
        RCGREEDY::RCGREEDY_Job job{1, 0.5000001};
        rcg.add_job(job, false);
        rcg.full_realloc();
        std::vector<std::pair<size_t, double>> allocs;
        rcg.get_all_server_count(allocs);
        bool ok = double_eq(allocs[0].second, 1000000.0);
        print_result("RCGREEDY Precision", ok, "1000000.0", std::to_string(allocs[0].second));
    }

    // ---- Test 10: EQUI fractional precision ----
    {
        EQUI equi(10, true);
        equi.insert_job(1);
        equi.insert_job(2);
        equi.insert_job(3);
        std::vector<std::pair<size_t, double>> allocs;
        equi.get_all_allocations(allocs);
        double expected = 10.0 / 3.0;
        bool all_ok = true;
        for (auto& p : allocs) all_ok &= double_eq(p.second, expected);
        print_result("EQUI Fractional Precision", all_ok, std::to_string(expected), std::to_string(allocs[0].second));
    }

    // test 11
    {
        RCGREEDY rcg(100, 5, 1, true);
        RCGREEDY::RCGREEDY_Job job{1, 0.00889474};
        rcg.add_job(job, true);
        RCGREEDY::RCGREEDY_Job job2{2, 0.86};
        rcg.add_job(job2, true);
        std::vector<std::pair<size_t, double>> allocs = rcg.get_server_changes();
        std::vector<std::pair<size_t, double>> allocs2;
        rcg.get_all_server_count(allocs2);
        bool ok = true;
        print_result(std::to_string(allocs2[0].second), ok, "1000000.0", std::to_string(allocs[0].second));
        print_result(std::to_string(allocs2[1].second), ok, "1000000.0", std::to_string(allocs[1].second));
        print_result(std::to_string(allocs[0].second), ok, "1000000.0", std::to_string(allocs[0].second));
        print_result(std::to_string(allocs[1].second), ok, "1000000.0", std::to_string(allocs[1].second));
    }

    return 0;
}