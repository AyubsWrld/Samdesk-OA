#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<int> read_line(std::istringstream& line) {
    std::vector<int> line_vals;
    int number{-1000};
    while (line >> number) {
        line_vals.push_back(number);
    }
    return line_vals;
}

std::vector<int> level_trend(const std::vector<int>& values) {
    std::vector<int> trend(values.size() - 1, -1000);
    std::transform(
        values.begin(), values.end() - 1,
        values.begin() + 1,
        trend.begin(),
        [](int lower, int upper) { return upper - lower; }
    );
    return trend;
}

auto check_numbers(const std::vector<int>& values, const int& overall_trend) {
    return std::adjacent_find(
        values.begin(), values.end(),
        [&overall_trend](int a, int b) {
            if (a == b || std::abs(a - b) > 3 || std::abs(a - b) < 1)
                return true;
            if ((overall_trend > 0 && a > b) || (overall_trend < 0 && a < b))
                return true;
            return false;
        }
    );
}

bool level_trend_is_valid(const std::vector<int>& values, bool problem_dampening) {
    std::vector<int> trend{level_trend(values)};

    const int general_trend = std::accumulate(
        trend.begin(), trend.end(), 0,
        [](int prev, int val) {
            if (val > 0) return prev + 1;
            if (val < 0) return prev - 1;
            return prev;
        }
    );

    if (!general_trend) return false;

    auto invalid_number_iter{check_numbers(values, general_trend)};
    if (invalid_number_iter == values.end()) return true;
    if (!problem_dampening) return false;

    const int index = std::distance(values.begin(), invalid_number_iter);

    std::vector<int> test_erase_current(values);
    if (index < (int)test_erase_current.size()) {
        test_erase_current.erase(test_erase_current.begin() + index);
        if (check_numbers(test_erase_current, general_trend) == test_erase_current.end())
            return true;
    }

    std::vector<int> test_erase_next(values);
    if (index + 1 < (int)test_erase_next.size()) {
        test_erase_next.erase(test_erase_next.begin() + index + 1);
        if (check_numbers(test_erase_next, general_trend) == test_erase_next.end())
            return true;
    }

    return false;
}

std::vector<bool> check_reactor_safety(const std::filesystem::path& input_file, bool problem_dampening) {
    std::vector<bool> safe_reports;
    std::ifstream read_in(input_file, std::ios::in);
    std::string line;

    while (std::getline(read_in, line)) {
        std::istringstream stream(line, std::ios::in);
        const std::vector<int> values{read_line(stream)};
        safe_reports.push_back(level_trend_is_valid(values, problem_dampening));
    }

    return safe_reports;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Expected input data file!\n";
        return 1;
    }

    const std::filesystem::path input_file{argv[1]};
    if (!std::filesystem::exists(input_file)) {
        throw std::runtime_error("File " + input_file.string() + " does not exist!");
    }

    const auto report_safety          = check_reactor_safety(input_file, false);
    const auto report_safety_dampened = check_reactor_safety(input_file, true);

    const int n_safe = std::count_if(report_safety.begin(), report_safety.end(),
                                     [](bool x) { return x; });
    const int n_safe_dampened = std::count_if(report_safety_dampened.begin(), report_safety_dampened.end(),
                                              [](bool x) { return x; });

    std::cout << "Safe reports:              " << n_safe << "\n";
    std::cout << "Safe reports (dampened):   " << n_safe_dampened << "\n";

    return 0;
}
