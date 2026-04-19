// Jordan Bankston - CS 405
// NumericOverflows_Enhanced.cpp
// Enhanced overflow/underflow demo with safer subtraction logic,
// structured test result storage, and summary reporting.

#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

// ------------------------------------------------------------
// Result helper to report success/failure while returning a value
// ------------------------------------------------------------
template <typename T>
struct OpResult {
    bool ok;
    T value;
};

// ------------------------------------------------------------
// Store test case results in a structured way
// ------------------------------------------------------------
struct TestCaseResult {
    std::string category;
    std::string type_name;
    std::string description;
    bool success;
    long double value;
};

// ------------------------------------------------------------
// Checked addition: returns {ok, value} without invoking UB
// Works for signed/unsigned integers and floating-point types
// ------------------------------------------------------------
template <typename T>
static OpResult<T> checked_add(T a, T b) {
    OpResult<T> r{ true, a };

    if constexpr (std::is_integral<T>::value) {
        if constexpr (std::is_unsigned<T>::value) {
            if (a > std::numeric_limits<T>::max() - b) {
                r.ok = false;
                return r;
            }
        } else {
            if (b > 0) {
                if (a > std::numeric_limits<T>::max() - b) {
                    r.ok = false;
                    return r;
                }
            } else if (b < 0) {
                if (a < std::numeric_limits<T>::min() - b) {
                    r.ok = false;
                    return r;
                }
            }
        }

        r.value = static_cast<T>(a + b);
        return r;
    } else {
        long double next = static_cast<long double>(a) + static_cast<long double>(b);

        if (!std::isfinite(next) ||
            next > static_cast<long double>(std::numeric_limits<T>::max()) ||
            next < static_cast<long double>(-std::numeric_limits<T>::max())) {
            r.ok = false;
            return r;
        }

        r.value = static_cast<T>(next);
        return r;
    }
}

// ------------------------------------------------------------
// Checked subtraction: separate logic so unsigned subtraction
// is handled safely without wraparound tricks
// ------------------------------------------------------------
template <typename T>
static OpResult<T> checked_sub(T a, T b) {
    OpResult<T> r{ true, a };

    if constexpr (std::is_integral<T>::value) {
        if constexpr (std::is_unsigned<T>::value) {
            if (a < b) {
                r.ok = false;
                return r;
            }
        } else {
            if (b > 0) {
                if (a < std::numeric_limits<T>::min() + b) {
                    r.ok = false;
                    return r;
                }
            } else if (b < 0) {
                if (a > std::numeric_limits<T>::max() + b) {
                    r.ok = false;
                    return r;
                }
            }
        }

        r.value = static_cast<T>(a - b);
        return r;
    } else {
        long double next = static_cast<long double>(a) - static_cast<long double>(b);

        if (!std::isfinite(next) ||
            next > static_cast<long double>(std::numeric_limits<T>::max()) ||
            next < static_cast<long double>(-std::numeric_limits<T>::max())) {
            r.ok = false;
            return r;
        }

        r.value = static_cast<T>(next);
        return r;
    }
}

// ------------------------------------------------------------
// add_numbers: start + (increment * steps) with protection
// ------------------------------------------------------------
template <typename T>
OpResult<T> add_numbers(const T& start, const T& increment, const unsigned long int& steps)
{
    OpResult<T> r{ true, start };

    for (unsigned long int i = 0; i < steps; ++i) {
        auto step = checked_add<T>(r.value, increment);
        if (!step.ok) {
            r.ok = false;
            return r;
        }
        r.value = step.value;
    }

    return r;
}

// ------------------------------------------------------------
// subtract_numbers: start - (decrement * steps) with protection
// ------------------------------------------------------------
template <typename T>
OpResult<T> subtract_numbers(const T& start, const T& decrement, const unsigned long int& steps)
{
    OpResult<T> r{ true, start };

    for (unsigned long int i = 0; i < steps; ++i) {
        auto step = checked_sub<T>(r.value, decrement);
        if (!step.ok) {
            r.ok = false;
            return r;
        }
        r.value = step.value;
    }

    return r;
}

// ------------------------------------------------------------
// Save structured test result
// ------------------------------------------------------------
void record_result(std::vector<TestCaseResult>& results,
                   const std::string& category,
                   const std::string& type_name,
                   const std::string& description,
                   bool success,
                   long double value)
{
    results.push_back({ category, type_name, description, success, value });
}

template <typename T>
void test_overflow(std::vector<TestCaseResult>& results)
{
    const unsigned long int steps = 5;
    const T increment = std::numeric_limits<T>::max() / steps;
    const T start = 0;

    std::string type_name = typeid(T).name();

    std::cout << "Overflow Test of Type = " << type_name << std::endl;

    std::cout << "\tAdding Numbers Without Overflow (" << +start << ", " << +increment << ", " << steps << ") = ";
    auto r1 = add_numbers<T>(start, increment, steps);
    if (r1.ok) {
        std::cout << +r1.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r1.value << " (status: false - prevented)" << std::endl;
    }
    record_result(results, "Overflow", type_name, "Without overflow", r1.ok, static_cast<long double>(r1.value));

    std::cout << "\tAdding Numbers With Overflow (" << +start << ", " << +increment << ", " << (steps + 1) << ") = ";
    auto r2 = add_numbers<T>(start, increment, steps + 1);
    if (r2.ok) {
        std::cout << +r2.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r2.value << " (status: false - overflow prevented)" << std::endl;
    }
    record_result(results, "Overflow", type_name, "With overflow", r2.ok, static_cast<long double>(r2.value));
}

template <typename T>
void test_underflow(std::vector<TestCaseResult>& results)
{
    const unsigned long int steps = 5;
    const T decrement = std::numeric_limits<T>::max() / steps;
    const T start = std::numeric_limits<T>::max();

    std::string type_name = typeid(T).name();

    std::cout << "Underflow Test of Type = " << type_name << std::endl;

    std::cout << "\tSubtracting Numbers Without Underflow (" << +start << ", " << +decrement << ", " << steps << ") = ";
    auto r1 = subtract_numbers<T>(start, decrement, steps);
    if (r1.ok) {
        std::cout << +r1.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r1.value << " (status: false - prevented)" << std::endl;
    }
    record_result(results, "Underflow", type_name, "Without underflow", r1.ok, static_cast<long double>(r1.value));

    std::cout << "\tSubtracting Numbers With Underflow (" << +start << ", " << +decrement << ", " << (steps + 1) << ") = ";
    auto r2 = subtract_numbers<T>(start, decrement, steps + 1);
    if (r2.ok) {
        std::cout << +r2.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r2.value << " (status: false - underflow prevented)" << std::endl;
    }
    record_result(results, "Underflow", type_name, "With underflow", r2.ok, static_cast<long double>(r2.value));
}

void do_overflow_tests(const std::string& star_line, std::vector<TestCaseResult>& results)
{
    std::cout << std::endl << star_line << std::endl;
    std::cout << "*** Running Overflow Tests ***" << std::endl;
    std::cout << star_line << std::endl;

    test_overflow<char>(results);
    test_overflow<wchar_t>(results);
    test_overflow<short int>(results);
    test_overflow<int>(results);
    test_overflow<long>(results);
    test_overflow<long long>(results);

    test_overflow<unsigned char>(results);
    test_overflow<unsigned short int>(results);
    test_overflow<unsigned int>(results);
    test_overflow<unsigned long>(results);
    test_overflow<unsigned long long>(results);

    test_overflow<float>(results);
    test_overflow<double>(results);
    test_overflow<long double>(results);
}

void do_underflow_tests(const std::string& star_line, std::vector<TestCaseResult>& results)
{
    std::cout << std::endl << star_line << std::endl;
    std::cout << "*** Running Underflow Tests ***" << std::endl;
    std::cout << star_line << std::endl;

    test_underflow<char>(results);
    test_underflow<wchar_t>(results);
    test_underflow<short int>(results);
    test_underflow<int>(results);
    test_underflow<long>(results);
    test_underflow<long long>(results);

    test_underflow<unsigned char>(results);
    test_underflow<unsigned short int>(results);
    test_underflow<unsigned int>(results);
    test_underflow<unsigned long>(results);
    test_underflow<unsigned long long>(results);

    test_underflow<float>(results);
    test_underflow<double>(results);
    test_underflow<long double>(results);
}

void print_summary(const std::vector<TestCaseResult>& results)
{
    int passed = 0;
    int prevented = 0;

    std::cout << "\n==================== SUMMARY ====================\n";

    for (const auto& result : results) {
        std::cout << result.category << " | "
                  << result.type_name << " | "
                  << result.description << " | "
                  << (result.success ? "Success" : "Prevented")
                  << " | value = " << result.value << '\n';

        if (result.success) {
            ++passed;
        } else {
            ++prevented;
        }
    }

    std::cout << "\nTotal Test Cases: " << results.size() << std::endl;
    std::cout << "Successful Operations: " << passed << std::endl;
    std::cout << "Prevented Operations: " << prevented << std::endl;
    std::cout << "=================================================\n";
}

int main()
{
    const std::string star_line(50, '*');
    std::vector<TestCaseResult> results;

    std::cout << "Starting Numeric Underflow / Overflow Tests!" << std::endl;

    do_overflow_tests(star_line, results);
    do_underflow_tests(star_line, results);
    print_summary(results);

    std::cout << std::endl << "All Numeric Underflow / Overflow Tests Complete!" << std::endl;

    return 0;
}
