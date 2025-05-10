
#include "unit_tests.hpp"
#include "experiments.hpp"


// Helper function prototypes
template<typename T>
bool get_arg(std::vector<std::string>& args, const std::string& flag, T& value);
bool get_arg(std::vector<std::string>& args, const std::string& flag, std::string& value);  // Specialization

// Template specialization for bool
template<>
bool get_arg<bool>(std::vector<std::string>& args, const std::string& flag, bool& value) {
    std::string str;
    if (!get_arg(args, flag, str)) return false;
    value = (str == "true" || str == "1");
    return true;
}

// Base template implementation
template<typename T>
bool get_arg(std::vector<std::string>& args, const std::string& flag, T& value) {
    auto it = std::find(args.begin(), args.end(), flag);
    if (it != args.end() && ++it != args.end()) {
        std::istringstream iss(*it);
        return iss >> value ? (args.erase(it - 1, it + 1), true) : false;
    }
    return false;
}

// Specialization for std::string
bool get_arg(std::vector<std::string>& args, const std::string& flag, std::string& value) {
    auto it = std::find(args.begin(), args.end(), flag);
    if (it != args.end() && ++it != args.end()) {
        value = *it;
        args.erase(it - 1, it + 1);
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " 0\n"
                  << "  " << argv[0] << " <flag> [experiment options]\n";
        return 1;
    }

    int main_flag = std::atoi(argv[1]);
    if (main_flag == 0) {
        unit_tests();
        return 0;
    }

    // Experiment mode
    std::vector<std::string> args(argv + 2, argv + argc);

    // Required parameters
    size_t trials = 0;
    int option = 0;
    std::string csv_output_file;
    bool generate_graphs = false;

    try {
        // Parse required arguments
        if (!get_arg(args, "--trials", trials) ||
            !get_arg(args, "--option", option) ||
            !get_arg(args, "--csv", csv_output_file) ||
            !get_arg(args, "--graphs", generate_graphs)) {
            throw std::runtime_error("Missing required parameters");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n"
                  << "Required parameters:\n"
                  << "  --trials <number>\n"
                  << "  --option <experiment-number>\n"
                  << "  --csv <filename>\n"
                  << "  --graphs <true/false>\n";
        return 1;
    }

    // Validate trials
    if (trials < 1) {
        std::cerr << "trials must be >= 1\n";
        return 1;
    }

    experiments(trials, option, csv_output_file, generate_graphs);
    return 0;
}

