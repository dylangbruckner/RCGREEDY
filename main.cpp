
#include "unit_tests.hpp"
#include "experiments.hpp"


// helper to parse arguments
bool get_arg(const std::vector<std::string>& args, const std::string& flag, std::string& value) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == flag && i + 1 < args.size()) {
            value = args[i + 1];
            return true;
        }
    }
    return false;
}

bool has_flag(const std::vector<std::string>& args, const std::string& flag) {
    for (const auto& arg : args) {
        if (arg == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " 0 [test_start test_end]\n"
                  << "  " << argv[0] << " <flag> [experiment options]\n";
        return 1;
    }

    int main_flag = std::atoi(argv[1]);
    if (main_flag == 0) {
        // Testing mode
        size_t test_start = 0, test_end = 0;
        if (argc >= 4) {
            test_start = std::stoul(argv[2]);
            test_end   = std::stoul(argv[3]);
        }
        unit_tests(test_start, test_end);
        return 0;
    }

    // Experiment mode
    std::vector<std::string> args(argv + 2, argv + argc);

    // Required experiment parameters
    std::string server_count_str, trials_str, job_size_lambda_str, arrival_lambda_str, data_tracking_flag_str;
    std::string csv_output_file;
    bool generate_graphs = false;

    // Parse required parameters
    if (!get_arg(args, "--server_count", server_count_str) ||
        !get_arg(args, "--trials", trials_str) ||
        !get_arg(args, "--job_size_lambda", job_size_lambda_str) ||
        !get_arg(args, "--arrival_lambda", arrival_lambda_str) ||
        !get_arg(args, "--data_tracking_flag", data_tracking_flag_str)) {
        std::cerr << "Missing required experiment parameters.\n";
        return 1;
    }

    // Optional parameters
    get_arg(args, "--csv_output_file", csv_output_file);
    generate_graphs = has_flag(args, "--generate_graphs");

    // Convert and validate parameters
    size_t server_count = std::stoul(server_count_str);
    size_t trials = std::stoul(trials_str);
    double job_size_lambda = std::stod(job_size_lambda_str);
    double arrival_lambda = std::stod(arrival_lambda_str);
    int data_tracking_flag = std::stoi(data_tracking_flag_str);

    if (server_count <= 1) {
        std::cerr << "server_count must be > 1.\n";
        return 1;
    }
    if (trials < 1) {
        std::cerr << "trials must be >= 1.\n";
        return 1;
    }

    experiments(server_count, trials, job_size_lambda, arrival_lambda,
                        data_tracking_flag, csv_output_file, generate_graphs);

    return 0;
}