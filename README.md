# Result.hpp

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
[![Header-Only](https://img.shields.io/badge/Header--only-lightgrey.svg)]()
![License](https://img.shields.io/badge/license-MIT-green.svg)

A single-header, lightweight C++20 implementation of a `Result` type for robust, exception-free error handling.

Inspired by Rust's `Result` type, this library leverages C++20 features (Concepts, `std::variant`) to provide a zero-cost abstraction for operations that may fail, without the overhead or unpredictability of `try-catch` blocks.

## Features

* **Header-Only:** No build steps or library linking required. Just drop it in.
* **C++20 Native:** Uses Concepts (`requires`) for clear compile-time constraints.
* **Zero-Cost Abstraction:** Heavily optimized with `[[nodiscard]]` and `constexpr` friendly structures.
* **Full Move Semantics:** Implements `&`, `const &`, `&&`, and `const &&` overloads for all monadic operations, allowing efficient chaining of move-only types (like `std::unique_ptr`).
* **In-Place Construction:** specialized factories (`ok_in_place`) to construct values directly inside the variant, avoiding unnecessary moves.
* **Monadic Interface:** Chain operations cleanly using `map`, `and_then`, `or_else`, and `match`.

## Requirements

* **C++20 Compiler:** A compiler supporting C++20 is required.
    * GCC 10+
    * Clang 12+
    * MSVC 19.29 (Visual Studio 2019 16.10) or later
* **GoogleTest (Optional):** Required only to build and run the provided test suite.

## Installation

This is a **header-only** library.

1.  Copy the `src/result.hpp` file into your project's include path.
2.  Include it in your code.

## Quick Start

### Basic Usage

Create functions that return a `Result` instead of throwing exceptions or returning magic numbers.

```c++
#include "src/result.hpp"
#include <string>
#include <stdexcept>

// A function that can fail
Result<double> safe_divide(double a, double b) {
    if (b == 0.0) {
        return Result<double>::err("Division by zero!", 1);
    } else {
        return Result<double>::ok(a / b);
    }
}

// A function that returns void on success
Result<void> print_if_positive(int x) {
    if (x < 0) {
        return Result<void>::err("Number is negative.");
    }
    std::cout << "Positive number: " << x << std::endl;
    return Result<void>::ok();
}
```

### Handling a Result

Check and unwrap the `Result` safely.

```c++
void handle_division() {
    // The 'if' check (safe)
    auto res = safe_divide(10.0, 2.0);
    if (res) { // 'res' is true if ok, false if err
        std::cout << "Result: " << *res << std::endl; // Use * or ->
    } else {
        std::cout << "Error: " << res.unwrap_err().message << std::endl;
    }

    // 2. The 'unwrap_or' (default value)
    double val = safe_divide(10.0, 0.0).unwrap_or(0.0);
    std::cout << "Value or default: " << val << std::endl; // Prints 0.0

    // The 'expect' (panic with message)
    // This will throw if it's an error
    try {
        double must_succeed = safe_divide(10.0, 0.0)
            .expect("Critical division failed");
    } catch (const std::runtime_error& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        // Prints: Caught exception: Critical division failed: Division by zero!
    }
}
```

### Chaining (The Monadic API)

The real power comes from chaining operations.

```c++
// Three functions that all return a Result
Result<std::string> get_input() {
    return Result<std::string>::ok("123");
    // return Result<std::string>::err("Failed to read input");
}

Result<int> parse_int(std::string s) {
    try {
        return Result<int>::ok(std::stoi(s));
    } catch (...) {
        return Result<int>::err("Not an integer");
    }
}

int double_value(int x) {
    return x * 2;
}

void process_chain() {
    auto final_result = get_input()
        .and_then(parse_int)     // If ok, calls parse_int(string)
        .map(double_value)       // If ok, maps the Result<int>
        .map([](int x) { return x + 1; }); // If ok, adds 1

    // If any step failed, 'final_result' is an Err.
    // If all succeeded, it's an Ok.

    if (final_result) {
        std::cout << "Final value: " << *final_result << std::endl;
        // Prints: Final value: 247
    } else {
        std::cout << "Chain failed: " << final_result.unwrap_err().message << std::endl;
    }
}
```

## API Reference (Brief)

`Result<T>`
- `Result<T>::ok(T value)` / `Result<T>::ok_in_place(...)`: Creates a success Result.
- `Result<T>::err(Error error)` / `Result<T>::err_in_place(const std::string& message, const int code = 0)`: Creates an error Result.
- `is_ok()`: Returns `true` if it's an `Ok` value.
- `is_err()`: Returns `true` if it's an `Error` value.
- `unwrap()`: Returns the `T` value. **Panics** if it's an `Error`.
- `expect(const std::string& msg)`: Like `unwrap()`, but **panics** with a custom message.
- `unwrap_err()`: Returns the `Error` value. **Panics** if it's an Ok.
- `expect_err(const std::string& msg)`: Like `unwrap_err()`, but **panics** with a custom message.
- `try_unwrap()`: Returns `T*` if `Ok`, `nullptr` if `Error`.
- `to_optional()`: Returns `std::optional<T>` (`T` if Ok, `std::nullopt` if `Error`).
- `unwrap_or(T default_val)`: Returns the `T` value or a default if it's an `Error`.
- `unwrap_or_else(Fn f)`: Returns the `T` value or computes it from the `Error` using `f`.
- `match(FOk ok_fn, FErr err_fn)` : Calls `ok_fn` with `T` value if it's an `Ok` or calls `err_fn` with `E` if it's an `Error` (`ok_fn` and `ok_err` must have the same return type)
- `map(Fn f)`: Calls `f` with the `T` value and returns a new Result with the new value.
- `map_err(Fn f)`: If `Error`, calls `f` with the `Error` to create a new `Error`.
- `and_then(Fn f)`: Calls `f` with the `T` value. `f` must return a Result.
- `or_else(Fn f)`: If `Error`, calls `f` with the `Error`. `f` must return a Result.

*All methods have &, const &, &&, and const && overloads for maximum performance and move-safety.*

## Building the Tests

The test suite is built with GoogleTest.

#### You can samply use the CMakeList.txt provided in the repo or do it by your self

1. Make sure you have GoogleTest available (e.g., via a submodule, find_package, or FetchContent).
2. If using CMake, you can use FetchContent to get GTest:
```cmake
# In your CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

# ...

enable_testing()
add_executable(run_tests tests/result_tests.cpp)
target_link_libraries(run_tests PRIVATE GTest::gtest_main)
target_include_directories(run_tests PRIVATE src)

include(GoogleTest)
gtest_discover_tests(run_tests)
```
3. Build and run the tests:
```shell
cmake -B build
cmake --build build
cd build
ctest
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.