# Result.hpp

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
[![Header-Only](https://img.shields.io/badge/Header--only-lightgrey.svg)]()
![License](https://img.shields.io/badge/license-MIT-green.svg)
![C++ CI](https://github.com/MohaIfk/result.hpp/actions/workflows/ci.yml/badge.svg)


A single-header, lightweight C++20 implementation of a `Result` type for robust, exception-free error handling.

Inspired by Rust's `Result` type, this library leverages C++20 features (Concepts, `std::variant`) to provide a zero-cost abstraction for operations that may fail, without the overhead or unpredictability of `try-catch` blocks.

## Features

* **Header-Only:** No build steps or library linking required. Just drop it in.
* **C++20 Native:** Uses Concepts (`requires`) for clear compile-time constraints.
* **Flexible Error** Types: `Result<T, E>` is generic over the error type `E`. It defaults to `Error (std::string message, int code)`, but you can use any type.
* **Zero-Cost Abstraction:** Heavily optimized with `[[nodiscard]]` and move semantics.
* **Full Move Semantics:** Implements `&`, `const &`, `&&`, and `const &&` overloads for all monadic operations, allowing efficient chaining of move-only types (like `std::unique_ptr`).
* **In-Place Construction:** Specialized factories (`ok_in_place`, `err_in_place`) to construct values directly inside the variant, avoiding unnecessary moves.
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

Create functions that return a `Result` instead of throwing exceptions or returning magic numbers. `Result<T>` is an alias for `Result<T, Error>`.

```c++
#include "result.hpp" // Assumes the file is in the include path
#include <string>
#include <iostream>
#include <stdexcept>

// A function that can fail
// By default, Result<double> is Result<double, Error>
Result<double> safe_divide(double a, double b) {
    if (b == 0.0) {
        // .err() with string and code is a helper for the default Error type
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

### Custom Error Type

You can specify any type as the error.

```c++
enum class FileError {
    NotFound,
    PermissionDenied,
    Unknown
};

Result<std::string, FileError> read_file(const std::string& path) {
    if (path.empty()) {
        return Result<std::string, FileError>::err(FileError::NotFound);
    }
    // logic to read file...
    return Result<std::string, FileError>::ok("File content");
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
        // .unwrap_err() gets the error value (Error struct by default)
        std::cout << "Error: " << res.unwrap_err().message << std::endl;
    }

    // The 'unwrap_or' (default value)
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

The class is `Result<T, E = Error>`. A specialization `Result<void, E>` is provided.

#### `Result<T, E>` (Value specialization)
- `static ok(T value)` / `static ok(U&& value)`: Creates a success Result from a value.
- `static ok_in_place(Args&&... args)`: Constructs T in-place.
- `static err(E error)`: Creates an error Result from an error value.
- `static err(const std::string&, int code = 0)`: Helper for `E = Error`.
- `static err_in_place(EArgs&&... args)`: Constructs `E` in-place.
- `is_ok()`: Returns `true` if it's an `Ok` value.
- `is_err()`: Returns `true` if it's an `Err` value.
- `operator bool()`: Same as `is_ok()`.
- `operator*()`: Dereferences to the contained `T` value. **Panics** if `Err`.
- `operator->()`: Dereferences to the contained `T` value. **Panics** if `Err`.
- `unwrap()`: Returns the `T` value. **Panics** if `Err`.
- `expect(const std::string& msg)`: Like `unwrap()`, but **panics** with a custom message.
- `unwrap_err()`: Returns the `E` value. **Panics** if `Ok`.
- `expect_err(const std::string& msg)`: Like `unwrap_err()`, but **panics** with a custom message.
- `try_unwrap()`: Returns `T*` if `Ok`, `nullptr` if `Err`.
- `to_optional()`: Returns `std::optional<T>` (`T` if `Ok`, `std::nullopt` if `Err`).
- `unwrap_or(T default_val)`: Returns the `T` value or `default_val` if `Err`.
- `unwrap_or_else(Fn<E> f)`: Returns the `T` value or computes it from the `E` value using `f`.
- `inspect(Fn<T> f)` : If it's `Ok` it calls `f` with `T` then return the same `Result<T, E>` in both cases.
- `inspect_err(Fn<E> f)` : If it's `Err` it calls `f` with `E` then return the same `Result<T, E>` in both cases.
- `contains(const U& value)`: Returns `true` if `Ok` and the `value` equals `value`.
- `match(FOk<T> ok_fn, FErr<E> err_fn)` : Calls `ok_fn` with `T` or `err_fn` with `E`. Both must return the same type.
- `map(Fn<T> f)`: Calls `f` with `T` and returns a `Result<U, E>` with the new value (or `Result<void, E>` if f returns void).
- `map_err(Fn<E> f)`: Calls `f` with `E` to create a new `E` and returns `Result<T, E>`.
- `and_then(Fn<T> f)`: Calls `f` with `T`. `f` must return a `Result<U, E>`.
- `or_else(Fn<E> f)`: If `Err`, calls `f` with `E`. `f` must return a `Result<T, E>`.

#### `Result<void, E>` (Void specialization)

- `static ok()`: Creates a success `Result<void, E>`.
- `static err(E error)` / `static err_in_place(...)` / `static err(const std::string&, int code = 0)`: Same as `Result<T, E>`.
- `is_ok()`, `is_err()`, `operator bool()`: Same as `Result<T, E>`.
- `unwrap()`: Does nothing. Panics if Err.
- `expect(const std::string& msg)`: Like `unwrap()`, but **panics** with a custom message.
- `unwrap_err()`, `expect_err(...)`: Same as `Result<T, E>`.
- `to_optional()`: Returns `std::optional<std::monostate>` (`{}` if `Ok`, `std::nullopt` if `Err`).
- `unwrap_or_else(Fn<E> f)`: If `Err`, calls `f(E)` which must return void.
- `inspect(Fn<T> f)` : If it's `Ok` it calls `f` then return the same `Result<void, E>` in both cases.
- `inspect_err(Fn<E> f)` : If it's `Err` it calls `f` with `E` then return the same `Result<void, E>` in both cases.
- `match(FOk ok_fn, FErr<E> err_fn)`: Calls `ok_fn()` or `err_fn(E)`.
- `map(Fn f)`: If `Ok`, calls `f()` and returns `Result<U, E>` (or `Result<void, E>` if `f` returns `void`).
- `map_err(Fn<E> f)`: Same as `Result<T, E>`, returns `Result<void, E>`.
- `and_then(Fn f)`: If `Ok`, calls `f()`. `f` must return a `Result<U, E>`.
- `or_else(Fn<E> f)`: If `Err`, calls `f(E)`. `f` must return a `Result<void, E>`.

*All methods have &, const &, &&, and const && overloads for maximum performance and move-safety.*

## Building the Tests

The test suite is built with GoogleTest.

#### You can samply use the CMakeList.txt provided in the repo or do it by your self

1. Make sure you have GoogleTest available (e.g., via a submodule, `find_package`, or `FetchContent`).
2. If using `CMake`, you can use `FetchContent` to get `GTest`:
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