// Jordan Bankston - CS 405
//NumericOverflows.cpp : safe overflow/underflow demo with detection & prevention

#include <iostream>      // std::cout
#include <limits>        // std::numeric_limits
#include <typeinfo>      // typeid
#include <cmath>         // std::isfinite, std::isinf
#include <type_traits>   // std::is_integral, std::is_floating_point

// ------------------------------------------------------------
// Result helper to report success/failure while returning a value
// ------------------------------------------------------------
template <typename T>
struct OpResult {
    bool ok;    // true  -> operation succeeded with no overflow/underflow
                // false -> operation was prevented (value is unchanged-from-last-safe step)
    T value;    // the computed value when ok==true; last safe value when ok==false
};

// ------------------------------------------------------------
// Checked addition: returns {ok,value} without invoking UB
// Works for signed/unsigned integers and floating-point types
// ------------------------------------------------------------
template <typename T>
static OpResult<T> checked_add(T a, T b) {
    OpResult<T> r{ true, a };

    if constexpr (std::is_integral<T>::value) {
        // Integer path: use max/min guards before the add
        if (b > 0) {
            if (a > std::numeric_limits<T>::max() - b) {
                r.ok = false;           // would overflow
                return r;
            }
        } else if (b < 0) {
            if (a < std::numeric_limits<T>::min() - b) {
                r.ok = false;           // would underflow (for signed types)
                return r;
            }
        }
        r.value = static_cast<T>(a + b);
        return r;
    } else {
        // Floating path: compute in wider precision and check finiteness
        auto next = static_cast<long double>(a) + static_cast<long double>(b);
        if (!std::isfinite(next) ||
            next > static_cast<long double>(std::numeric_limits<T>::max()) ||
            next < static_cast<long double>(-std::numeric_limits<T>::max())) {
            r.ok = false;               // would overflow to ±inf or beyond range
            return r;
        }
        r.value = static_cast<T>(next);
        return r;
    }
}

// ------------------------------------------------------------
// Checked subtraction: same idea as above, but for a - b
// ------------------------------------------------------------
template <typename T>
static OpResult<T> checked_sub(T a, T b) {
    // a - b  == a + (-b)  → reuse checked_add
    return checked_add<T>(a, static_cast<T>(-b));
}

// ------------------------------------------------------------
// add_numbers: start + (increment * steps) with protection
// ------------------------------------------------------------
template <typename T>
OpResult<T> add_numbers(T const& start, T const& increment, unsigned long int const& steps)
{
    OpResult<T> r{ true, start };

    for (unsigned long int i = 0; i < steps; ++i) {
        auto step = checked_add<T>(r.value, increment);
        if (!step.ok) {
            r.ok = false;            // signal failure; r.value remains last safe value
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
OpResult<T> subtract_numbers(T const& start, T const& decrement, unsigned long int const& steps)
{
    OpResult<T> r{ true, start };

    for (unsigned long int i = 0; i < steps; ++i) {
        auto step = checked_sub<T>(r.value, decrement);
        if (!step.ok) {
            r.ok = false;            // signal failure; r.value remains last safe value
            return r;
        }
        r.value = step.value;
    }
    return r;
}

//  NOTE:
//    You will see the unary ('+') operator used in front of the variables in the test_XXX methods.
//    This forces the output to be a number for cases where cout would assume it is a character.

template <typename T>
void test_overflow()
{
    // START DO NOT CHANGE
    const unsigned long int steps = 5;
    const T increment = std::numeric_limits<T>::max() / steps;
    const T start = 0;

    std::cout << "Overflow Test of Type = " << typeid(T).name() << std::endl;
    // END DO NOT CHANGE

    // No-overflow case
    std::cout << "\tAdding Numbers Without Overflow (" << +start << ", " << +increment << ", " << steps << ") = ";
    auto r1 = add_numbers<T>(start, increment, steps);
    if (r1.ok) {
        std::cout << +r1.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r1.value << " (status: false — prevented)" << std::endl;
    }

    // Intentional overflow case
    std::cout << "\tAdding Numbers With Overflow (" << +start << ", " << +increment << ", " << (steps + 1) << ") = ";
    auto r2 = add_numbers<T>(start, increment, steps + 1);
    if (r2.ok) {
        std::cout << +r2.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r2.value << " (status: false — overflow prevented)" << std::endl;
    }
}

template <typename T>
void test_underflow()
{
    // START DO NOT CHANGE
    const unsigned long int steps = 5;
    const T decrement = std::numeric_limits<T>::max() / steps;
    const T start = std::numeric_limits<T>::max();

    std::cout << "Underflow Test of Type = " << typeid(T).name() << std::endl;
    // END DO NOT CHANGE

    // No-underflow case
    std::cout << "\tSubtracting Numbers Without Overflow (" << +start << ", " << +decrement << ", " << steps << ") = ";
    auto r1 = subtract_numbers<T>(start, decrement, steps);
    if (r1.ok) {
        std::cout << +r1.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r1.value << " (status: false — prevented)" << std::endl;
    }

    // Intentional underflow case
    std::cout << "\tSubtracting Numbers With Overflow (" << +start << ", " << +decrement << ", " << (steps + 1) << ") = ";
    auto r2 = subtract_numbers<T>(start, decrement, steps + 1);
    if (r2.ok) {
        std::cout << +r2.value << " (status: true)" << std::endl;
    } else {
        std::cout << +r2.value << " (status: false — underflow prevented)" << std::endl;
    }
}

void do_overflow_tests(const std::string& star_line)
{
    std::cout << std::endl << star_line << std::endl;
    std::cout << "*** Running Overflow Tests ***" << std::endl;
    std::cout << star_line << std::endl;

    // signed integers
    test_overflow<char>();
    test_overflow<wchar_t>();
    test_overflow<short int>();
    test_overflow<int>();
    test_overflow<long>();
    test_overflow<long long>();

    // unsigned integers
    test_overflow<unsigned char>();
    test_overflow<unsigned short int>();
    test_overflow<unsigned int>();
    test_overflow<unsigned long>();
    test_overflow<unsigned long long>();

    // real numbers
    test_overflow<float>();
    test_overflow<double>();
    test_overflow<long double>();
}

void do_underflow_tests(const std::string& star_line)
{
    std::cout << std::endl << star_line << std::endl;
    std::cout << "*** Running Undeflow Tests ***" << std::endl;
    std::cout << star_line << std::endl;

    // signed integers
    test_underflow<char>();
    test_underflow<wchar_t>();
    test_underflow<short int>();
    test_underflow<int>();
    test_underflow<long>();
    test_underflow<long long>();

    // unsigned integers
    test_underflow<unsigned char>();
    test_underflow<unsigned short int>();
    test_underflow<unsigned int>();
    test_underflow<unsigned long>();
    test_underflow<unsigned long long>();

    // real numbers
    test_underflow<float>();
    test_underflow<double>();
    test_underflow<long double>();
}

/// Entry point
int main()
{
    const std::string star_line = std::string(50, '*');

    std::cout << "Starting Numeric Underflow / Overflow Tests!" << std::endl;

    do_overflow_tests(star_line);
    do_underflow_tests(star_line);

    std::cout << std::endl << "All Numeric Underflow / Overflow Tests Complete!" << std::endl;

    return 0;
}
