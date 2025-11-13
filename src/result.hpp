//
// result.hpp — A C++20 Result type
//
// Copyright (c) 2025 Mohammed Ifkirne
// <mohammedifkirne569@gmail.com> - <https://github.com/MohaIfk/>
//
// Distributed under the MIT License.
// See the accompanying LICENSE file for details.
// (Or visit https://opensource.org/licenses/MIT)
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// A lightweight, header-only, C++20 Result type for robust,
// exception-free error handling.
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#pragma once

#if _MSC_VER
#include <yvals.h>
#endif

#if __cplusplus >= 202002L || _HAS_CXX20

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <functional>
#include <utility> // For std::in_place_index

/**
 * @struct Error
 * @brief A simple struct to hold an error message and an optional error code.
 *
 * This is the default error type for the Result class. It provides a human-readable
 * message and a machine-readable code to describe an error condition.
 */
struct Error {
    std::string message; ///< The error message.
    int code = 0; ///< The error code.

    /**
     * @brief Compares two Error objects for equality.
     * @param other The other Error object to compare against.
     * @return True if both the message and code are equal, false otherwise.
     */
    bool operator==(const Error& other) const {
        return message == other.message && code == other.code;
    }
};

/**
 * @struct Ok
 * @brief A wrapper for the success value of a Result.
 * @tparam T The type of the success value.
 */
template <typename T>
struct Ok {
    T value; ///< The success value.

    /**
     * @brief Constructs an Ok object in-place from the given arguments.
     * @tparam Args The types of the arguments.
     * @param args The arguments to forward to the constructor of T.
     */
    template<typename... Args>
        requires std::constructible_from<T, Args...>
    explicit(sizeof...(Args) == 1) Ok(Args&&... args)
        : value(std::forward<Args>(args)...) {}
};

/**
 * @struct Err
 * @brief A wrapper for the error value of a Result.
 * @tparam E The type of the error value.
 */
template <typename E>
struct Err {
    E error; ///< The error value.

    /**
     * @brief Constructs an Err object in-place from the given arguments.
     * @tparam Args The types of the arguments.
     * @param args The arguments to forward to the constructor of E.
     */
    template<typename... Args>
        requires std::constructible_from<E, Args...>
    explicit(sizeof...(Args) == 1) Err(Args&&... args)
        : error(std::forward<Args>(args)...) {}
};

/**
 * @class Result
 * @brief A type that represents either a success (Ok) or a failure (Err).
 *
 * `Result<T, E>` is a type that can hold either a value of type `T` (representing success)
 * or a value of type `E` (representing an error). This allows for explicit, robust
 * error handling without resorting to exceptions.
 *
 * The default error type `E` is `Error`. A specialization `Result<void, E>` is provided
 * for operations that do not produce a value on success.
 *
 * @tparam T The type of the success value.
 * @tparam E The type of the error value. Defaults to `Error`.
 */
template<typename T, typename E = Error>
class Result;
/// @brief Specialization of the Result class for operations that do not return a value on success.
template<typename E>
class Result<void, E>;

/// @brief A type trait to check if a type is a Result.
template<typename>
struct is_result : std::false_type {};
/// @brief A type trait to check if a type is a Result.
template<typename T, typename E>
struct is_result<Result<T, E>> : std::true_type {};
/// @brief A convenience variable template for the is_result type trait.
template<typename T>
inline constexpr bool is_result_v = is_result<T>::value;

/**
 * @concept returns_result_for
 * @brief A concept that checks if a function returns a `Result` type when called with the given arguments.
 *
 * This concept is used to constrain template parameters to ensure that a given
 * function is invocable with a set of arguments and that its return type is a specialization
 * of the `Result` class.
 *
 * @tparam F The callable type to check.
 * @tparam Args The types of the arguments to invoke the callable with.
 */
template<typename F, typename... Args>
concept returns_result_for = std::invocable<F, Args...> && is_result_v<std::invoke_result_t<F, Args...>>;

// Specialization for Result<void>
template<typename E>
class Result<void, E> {
public:
    using value_type = void; ///< The type of the success value (void).
    using error_type = E; ///< The type of the error value.

    /**
     * @brief Creates a new `Result<void, E>` in a success (Ok) state.
     * @return A new `Result<void, E>` containing an Ok value.
     */
    [[nodiscard]] static Result<void, E> ok() {
        return Result<void, E>(std::in_place_index<0>);
    }

    /**
     * @brief Creates a new `Result<void, E>` in an error (Err) state.
     * @param error The error value to move into the new `Result`.
     * @return A new `Result<void, E>` containing the given error.
     */
    [[nodiscard]] static Result<void, E> err(E error) {
        return Result<void, E>(std::in_place_index<1>, std::move(error));
    }

    /**
     * @brief A convenience factory for creating an `Err` from a message and code.
     *
     * This function is only available when `E` is the default `Error` type.
     *
     * @param message The error message.
     * @param code An optional error code.
     * @return A new `Result<void, E>` containing an `Err` value.
     */
    [[nodiscard]] static Result<void, E> err(const std::string& message, const int code = 0)
        requires std::is_same_v<E, Error>
    {
        return Result<void, E>(std::in_place_index<1>, Error{message, code});
    }

    /**
     * @brief Creates a new `Result<void, E>` with an in-place constructed `Err` value.
     * @tparam EArgs The types of the arguments to forward to `E`'s constructor.
     * @param args The arguments to forward to the constructor of `E`.
     * @return A new `Result<void, E>` containing an in-place constructed `Err` value.
     */
    template<typename... EArgs>
        requires std::constructible_from<E, EArgs...>
    [[nodiscard]] static Result<void, E> err_in_place(EArgs&&... args) {
        return Result<void, E>(std::in_place_index<1>, std::forward<EArgs>(args)...);
    }

    /**
     * @brief Checks if the `Result` is an `Ok` value.
     * @return `true` if the `Result` is `Ok`, `false` otherwise.
     */
    [[nodiscard]] bool is_ok() const noexcept { return std::holds_alternative<std::monostate>(data_); }

    /**
     * @brief Checks if the `Result` is an `Err` value.
     * @return `true` if the `Result` is `Err`, `false` otherwise.
     */
    [[nodiscard]] bool is_err() const noexcept { return std::holds_alternative<E>(data_); }

    /**
     * @brief Converts the `Result` to a boolean.
     * @return `true` if the `Result` is `Ok`, `false` otherwise.
     */
    explicit operator bool() const noexcept { return is_ok(); }


    /**
     * @brief Does nothing if the `Result` is `Ok`, otherwise throws a `std::runtime_error` with a custom message.
     * @param msg The message to include in the exception.
     * @throw std::runtime_error if the `Result` is an `Err`.
     */
    void expect(const std::string& msg) & {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }
    /// @copydoc expect
    void expect(const std::string& msg) const & {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }
    /// @copydoc expect
    void expect(const std::string& msg) && {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }
    /// @copydoc expect
    void expect(const std::string& msg) const && {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }

    /**
     * @brief Does nothing if the `Result` is `Ok`, otherwise throws a `std::runtime_error`.
     * @throw std::runtime_error if the `Result` is an `Err`.
     */
    void unwrap() & {
        expect("Attempted to unwrap error result");
    }
    /// @copydoc unwrap
    void unwrap() const & {
        expect("Attempted to unwrap error result");
    }
    /// @copydoc unwrap
    void unwrap() && {
        std::move(*this).expect("Attempted to unwrap error result");
    }
    /// @copydoc unwrap
    void unwrap() const && {
        std::move(*this).expect("Attempted to unwrap error result");
    }

    /**
     * @brief Returns the contained `Err` value.
     * @return A reference to the contained `Err` value.
     * @throw std::runtime_error if the `Result` is an `Ok`.
     */
    [[nodiscard]] E& unwrap_err() & {
        return std::visit(
            [&]<typename T0>(T0& arg) -> E& {
                if constexpr (std::is_same_v<std::decay_t<T0>, E>) {
                    return arg;
                } else {
                    throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        data_);
    }
    /// @copydoc unwrap_err
    [[nodiscard]] const E& unwrap_err() const & {
        return std::visit(
            [&]<typename T0>(const T0& arg) -> const E& {
                if constexpr (std::is_same_v<std::decay_t<T0>, E>) {
                    return arg;
                } else {
                    throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        data_);
    }
    /// @copydoc unwrap_err
    [[nodiscard]] E&& unwrap_err() && {
        return std::visit(
            [&]<typename T0>(T0&& arg) -> E&& {
                if constexpr (std::is_same_v<std::decay_t<T0>, E>) {
                    return std::move(arg);
                } else {
                    throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        std::move(data_));
    }
    /// @copydoc unwrap_err
    [[nodiscard]] const E&& unwrap_err() const && {
        return std::visit(
            [&]<typename T0>(const T0&& arg) -> const E&& {
                if constexpr (std::is_same_v<std::decay_t<T0>, E>) {
                    return std::move(arg);
                } else {
                    throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        std::move(data_));
    }

    /**
     * @brief Returns the contained `Err` value, otherwise throws a `std::runtime_error` with a custom message.
     * @param msg The message to include in the exception.
     * @return A reference to the contained `Err` value.
     * @throw std::runtime_error if the `Result` is an `Ok`.
     */
    [[nodiscard]] E& expect_err(const std::string& msg) & {
        if (is_err()) return std::get<E>(data_);
        throw std::runtime_error(msg);
    }
    /// @copydoc expect_err
    [[nodiscard]] const E& expect_err(const std::string& msg) const & {
        if (is_err()) return std::get<E>(data_);
        throw std::runtime_error(msg);
    }
    /// @copydoc expect_err
    [[nodiscard]] E&& expect_err(const std::string& msg) && {
        if (is_err()) return std::move(std::get<E>(data_));
        throw std::runtime_error(msg);
    }
    /// @copydoc expect_err
    [[nodiscard]] const E&& expect_err(const std::string& msg) const && {
        if (is_err()) return std::move(std::get<E>(data_));
        throw std::runtime_error(msg);
    }

    /**
     * @brief Applies a function to the contained value (if any), or a fallback function to the error.
     *
     * This function is used to handle both `Ok` and `Err` cases in a single call.
     * Both functions must return the same type.
     *
     * @tparam FOk The type of the function to apply to the `Ok` value.
     * @tparam FErr The type of the function to apply to the `Err` value.
     * @param ok_fn The function to call if the `Result` is `Ok`.
     * @param err_fn The function to call if the `Result` is `Err`.
     * @return The value returned by either `ok_fn` or `err_fn`.
     */
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, E&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    /// @copydoc match
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, const E&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, const E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    /// @copydoc match
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, E&&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    /// @copydoc match
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, const E&&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, const E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }

    /**
     * @brief Does nothing if `Ok`, otherwise calls a function with the `Err` value.
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     */
    template<typename F> requires std::invocable<F, E&> && std::is_void_v<std::invoke_result_t<F, E&>>
    void unwrap_or_else(F&& f) & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<E>(data_));
        }
    }
    /// @copydoc unwrap_or_else
    template<typename F> requires std::invocable<F, const E&> && std::is_void_v<std::invoke_result_t<F, const E&>>
    void unwrap_or_else(F&& f) const & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<E>(data_));
        }
    }
    /// @copydoc unwrap_or_else
    template<typename F> requires std::invocable<F, E&&> && std::is_void_v<std::invoke_result_t<F, E&&>>
    void unwrap_or_else(F&& f) && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<E>(data_)));
        }
    }
    /// @copydoc unwrap_or_else
    template<typename F> requires std::invocable<F, const E&&> && std::is_void_v<std::invoke_result_t<F, const E&&>>
    void unwrap_or_else(std::function<void(E&&)> f) && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<E>(data_)));
        }
    }

    /**
     * @brief Calls a function with the `Ok` value if the `Result` is `Ok`.
     * @tparam F The type of the function to call.
     * @param f The function to call.
     * @return A reference to the `Result`.
     */
    template<std::invocable F>
    Result& inspect(F&& f) & {
        if (is_ok()) {
            std::invoke(std::forward<F>(f));
        }
        return *this;
    }
    /// @copydoc inspect
    template<std::invocable F>
    const Result& inspect(F&& f) const & {
        if (is_ok()) {
            std::invoke(std::forward<F>(f));
        }
        return *this;
    }
    /// @copydoc inspect
    template<std::invocable F>
    Result&& inspect(F&& f) && {
        if (is_ok()) {
            std::invoke(std::forward<F>(f));
        }
        return std::move(*this);
    }
    /// @copydoc inspect
    template<std::invocable F>
    const Result&& inspect(F&& f) const && {
        if (is_ok()) {
            std::invoke(std::forward<F>(f));
        }
        return std::move(*this);
    }

    /**
     * @brief Calls a function with the `Err` value if the `Result` is `Err`.
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return A reference to the `Result`.
     */
    template<std::invocable<E&> F>
    Result& inspect_err(F&& f) & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<E>(data_));
        }
        return *this;
    }
    /// @copydoc inspect_err
    template<std::invocable<const E&> F>
    const Result& inspect_err(F&& f) const & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<E>(data_));
        }
        return *this;
    }
    /// @copydoc inspect_err
    template<std::invocable<E&&> F>
    Result&& inspect_err(F&& f) && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<E>(data_)));
        }
        return std::move(*this);
    }
    /// @copydoc inspect_err
    template<std::invocable<const E&&> F>
    const Result&& inspect_err(F&& f) const && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<E>(data_)));
        }
        return std::move(*this);
    }

    /**
     * @brief Chains a computation that returns a `Result`.
     *
     * If the `Result` is `Ok`, calls `f` and returns the `Result`.
     * If the `Result` is `Err`, returns the `Err` value.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call.
     * @return The `Result` returned by `f`, or the original `Err` value.
     */
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    /// @copydoc and_then
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) const & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    /// @copydoc and_then
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc and_then
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) const && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Maps a `Result<void, E>` to a `Result<U, E>` by applying a function to the `Ok` value.
     *
     * If the `Result` is `Ok`, calls `f` and returns a new `Result` with the result of `f`.
     * If the `Result` is `Err`, returns the `Err` value.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call.
     * @return A new `Result` with the value returned by `f`, or the original `Err` value.
     */
    template<typename F> requires std::invocable<F>
    auto map(F&& f) & {
        return map_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map
    template<typename F> requires std::invocable<F>
    auto map(F&& f) const & {
        return map_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map
    template<typename F> requires std::invocable<F>
    auto map(F&& f) && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc map
    template<typename F> requires std::invocable<F>
    auto map(F&& f) const && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Maps a `Result<void, E>` to a `Result<void, F>` by applying a function to the `Err` value.
     *
     * If the `Result` is `Err`, calls `f` with the `Err` value and returns a new `Result` with the result of `f`.
     * If the `Result` is `Ok`, returns the `Ok` value.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return A new `Result` with the error value returned by `f`, or the original `Ok` value.
     */
    template<typename F> requires std::invocable<F, E&>
    auto map_err(F&& f) & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map_err
    template<typename F> requires std::invocable<F, const E&>
    auto map_err(F&& f) const & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map_err
    template<typename F> requires std::invocable<F, E&&>
    auto map_err(F&& f) && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc map_err
    template<typename F> requires std::invocable<F, const E&&>
    auto map_err(F&& f) const && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Returns the `Result` if it is `Ok`, otherwise calls `f` with the `Err` value and returns the result.
     *
     * This is useful for error recovery.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return The original `Result` if `Ok`, or the `Result` returned by `f`.
     */
    template<typename F> requires returns_result_for<F, E&>
    Result<void, E> or_else(F&& f) & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    /// @copydoc or_else
    template<typename F> requires returns_result_for<F, const E&>
    Result<void, E> or_else(F&& f) const & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    /// @copydoc or_else
    template<typename F> requires returns_result_for<F, E&&>
    Result<void, E> or_else(F&& f) && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc or_else
    template<typename F> requires returns_result_for<F, const E&&>
    Result<void, E> or_else(F&& f) const && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Converts the `Result` to a `std::optional<std::monostate>`.
     * @return `std::monostate` if `Ok`, `std::nullopt` if `Err`.
     */
    [[nodiscard]] std::optional<std::monostate> to_optional() const noexcept {
        return std::visit(
            [&]<typename T0>(T0 &&) -> std::optional<std::monostate> {
                if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                    return std::monostate{};
                } else {
                    return std::nullopt;
                }
            },
          data_
        );
    }

private:
    /// @brief Private constructor for Ok results.
    explicit Result(std::in_place_index_t<0>) : data_(std::in_place_index<0>) {}

    /// @brief Private constructor for Err results.
    template<typename... Args>
    explicit Result(std::in_place_index_t<1>, Args&&... args) : data_(std::in_place_index<1>, std::forward<Args>(args)...) {}

    /// @brief The underlying variant that holds either a `std::monostate` (for Ok) or an `E` (for Err).
    std::variant<std::monostate, E> data_;

    /// @brief Implementation of the `match` method.
    /// @brief Implementation of the `match` method.
    template <typename Self, typename FOk, typename FErr>
    static auto match_impl(Self&& self, FOk&& ok_fn, FErr&& err_fn) {
        using ResultType = std::invoke_result_t<FOk>;

        return std::visit(
            [&]<typename T0>(T0&& arg) -> ResultType {
                if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                    return std::invoke(std::forward<FOk>(ok_fn));
                } else {
                    return std::invoke(std::forward<FErr>(err_fn), std::forward<T0>(arg));
                }
            }, std::forward<Self>(self).data_);
    }

    /// @brief Implementation of the `map` method.
    template <typename Self, typename F>
    static auto map_impl(Self&& self, F&& f) {
        using ResultType = std::invoke_result_t<F>;

        if constexpr (std::is_void_v<ResultType>) {
            return std::visit(
                [&]<typename T0>(T0&& arg) -> Result<void, E> {
                    if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                        std::invoke(std::forward<F>(f));
                        return Result<void, E>::ok();
                    } else {
                        return Result<void, E>::err(std::forward<T0>(arg));
                    }
                }, std::forward<Self>(self).data_);
        }
        else {
            return std::visit(
                [&]<typename T0>(T0&& arg) -> Result<ResultType, E> {
                    if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                        return Result<ResultType, E>::ok(
                            std::invoke(std::forward<F>(f))
                        );
                    } else {
                        return Result<ResultType, E>::err(std::forward<T0>(arg));
                    }
                }, std::forward<Self>(self).data_);
        }
    }

    /// @brief Implementation of the `and_then` method.
    template <typename Self, typename F>
    static auto and_then_impl(Self&& self, F&& f) {
        using ResultType = std::invoke_result_t<F>;

        return std::visit(
            [&]<typename T0>(T0&& arg) -> ResultType {
                if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                  return std::invoke(std::forward<F>(f));
                } else {
                    return ResultType::err(std::forward<T0>(arg));
                }
            },
            std::forward<Self>(self).data_
        );
    }

    /// @brief Implementation of the `map_err` method.
    template <typename Self, typename F>
    static auto map_err_impl(Self&& self, F&& f) {
        using ErrorType = std::invoke_result_t<F, decltype(std::forward<Self>(self).unwrap_err())>;

        return std::visit(
            [&]<typename T0>(T0&& arg) -> Result<void, ErrorType> {
                if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                    return Result<void, ErrorType>::ok();
                } else {
                    using FResult = std::invoke_result_t<F, T0>;
                    if constexpr (std::is_same_v<FResult, E>) {
                        return Result<void, ErrorType>::err(std::invoke(std::forward<F>(f), std::forward<T0>(arg)));
                    } else {
                      return Result<void, ErrorType>::err_in_place(std::invoke(std::forward<F>(f), std::forward<T0>(arg)));
                    }
                }
            },
            std::forward<Self>(self).data_
        );
    }

    /// @brief Implementation of the `or_else` method.
    template <typename Self, typename F>
    static Result<void, E> or_else_impl(Self&& self, F&& f) {
        return std::visit(
            [&]<typename T0>(T0&& arg) -> Result<void, E> {
                if constexpr (std::is_same_v<std::decay_t<T0>, std::monostate>) {
                    return Result<void, E>::ok();
                } else {
                    return std::invoke(std::forward<F>(f), std::forward<T0>(arg));
                }
            },
            std::forward<Self>(self).data_
        );
    }

    template <typename, typename>
    friend class Result;
};

template<typename T, typename E>
class Result {
public:
    using value_type = T; ///< The type of the success value.
    using error_type = E; ///< The type of the error value.

    /**
     * @brief Creates a new `Result` in a success (Ok) state.
     * @param value The value to move into the new `Result`.
     * @return A new `Result` containing the given value.
     */
    [[nodiscard]] static Result<T, E> ok(T value) {
        return Result<T, E>(std::in_place_index<0>, Ok<T>{std::move(value)});
    }

    /**
     * @brief Creates a new `Result` in a success (Ok) state from a convertible value.
     * @tparam U The type of the value to convert from.
     * @param value The value to convert and move into the new `Result`.
     * @return A new `Result` containing the converted value.
     */
    template<typename U>
        requires std::constructible_from<T, U>
    [[nodiscard]] static Result<T, E> ok(U&& value) {
        return Result<T, E>(std::in_place_index<0>, T(std::forward<U>(value)));
    }

    /**
     * @brief Creates a new `Result` with an in-place constructed `Ok` value.
     * @tparam Args The types of the arguments to forward to `T`'s constructor.
     * @param args The arguments to forward to the constructor of `T`.
     * @return A new `Result` containing an in-place constructed `Ok` value.
     */
    template<typename... Args>
        requires std::constructible_from<T, Args...>
    [[nodiscard]] static Result<T, E> ok_in_place(Args&&... args) {
      return Result<T, E>(std::in_place_index<0>, Ok<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief Creates a new `Result` in an error (Err) state.
     * @param error The error value to move into the new `Result`.
     * @return A new `Result` containing the given error.
     */
    [[nodiscard]] static Result<T, E> err(E error) {
        return Result<T, E>(std::in_place_index<1>, Err<E>{std::move(error)});
    }

    /**
     * @brief A convenience factory for creating an `Err` from a message and code.
     *
     * This function is only available when `E` is the default `Error` type.
     *
     * @param message The error message.
     * @param code An optional error code.
     * @return A new `Result` containing an `Err` value.
     */
    [[nodiscard]] static Result<T, E> err(const std::string &message, const int code = 0)
        requires std::is_same_v<E, Error>
    {
        return Result<T, E>(std::in_place_index<1>, Err<Error>(message, code));
    }

    /**
     * @brief Creates a new `Result` with an in-place constructed `Err` value.
     * @tparam EArgs The types of the arguments to forward to `E`'s constructor.
     * @param args The arguments to forward to the constructor of `E`.
     * @return A new `Result` containing an in-place constructed `Err` value.
     */
    template<typename... EArgs>
        requires std::constructible_from<E, EArgs...>
    [[nodiscard]] static Result<T, E> err_in_place(EArgs&&... args) {
      return Result<T, E>(std::in_place_index<1>, Err<E>(std::forward<EArgs>(args)...));
    }

    /**
     * @brief Checks if the `Result` is an `Ok` value.
     * @return `true` if the `Result` is `Ok`, `false` otherwise.
     */
    [[nodiscard]] bool is_ok() const { return std::holds_alternative<Ok<T>>(data_);  }

    /**
     * @brief Checks if the `Result` is an `Err` value.
     * @return `true` if the `Result` is `Err`, `false` otherwise.
     */
    [[nodiscard]] bool is_err() const { return std::holds_alternative<Err<E>>(data_); }

    /**
     * @brief Converts the `Result` to a boolean.
     * @return `true` if the `Result` is `Ok`, `false` otherwise.
     */
    explicit operator bool() const noexcept { return is_ok(); }

    /**
     * @brief Accesses the contained value via a pointer.
     * @return A pointer to the contained value.
     * @throw std::runtime_error if the `Result` is an `Err`.
     */
    T* operator->() & { return &unwrap(); }
    /// @copydoc operator->
    const T* operator->() const & { return &unwrap(); }

    /**
     * @brief Accesses the contained value.
     * @return A reference to the contained value.
     * @throw std::runtime_error if the `Result` is an `Err`.
     */
    T& operator*() & { return unwrap(); }
    /// @copydoc operator*
    const T& operator*() const & { return unwrap(); }
    /// @copydoc operator*
    T&& operator*() && { return std::move(unwrap()); }
    /// @copydoc operator*
    const T&& operator*() const && { return std::move(unwrap()); }

    /**
     * @brief Returns the contained `Ok` value, otherwise throws a `std::runtime_error` with a custom message.
     * @param msg The message to include in the exception.
     * @return A reference to the contained `Ok` value.
     * @throw std::runtime_error if the `Result` is an `Err`.
     */
    T& expect(const std::string& msg) & {
        return std::visit(
            [&]<typename T0>(T0& arg) -> T& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return arg.value;
                } else if constexpr (std::is_same_v<std::decay_t<T0>, Err<Error>>) {
                    throw std::runtime_error(msg + ": " + arg.error.message);
                } else {
                    throw std::runtime_error(msg);
                }
            }, data_);
    }
    /// @copydoc expect
    const T& expect(const std::string& msg) const & {
        return std::visit(
            [&]<typename T0>(const T0& arg) -> const T& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return arg.value;
                } else if constexpr (std::is_same_v<std::decay_t<T0>, Err<Error>>) {
                    throw std::runtime_error(msg + ": " + arg.error.message);
                } else {
                    throw std::runtime_error(msg);
                }
            }, data_);
    }
    /// @copydoc expect
    T&& expect(const std::string& msg) && {
        return std::visit(
            [&]<typename T0>(T0&& arg) -> T&& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return std::move(arg.value);
                } else if constexpr (std::is_same_v<std::decay_t<T0>, Err<Error>>) {
                    throw std::runtime_error(msg + ": " + arg.error.message);
                } else {
                    throw std::runtime_error(msg);
                }
            }, std::move(data_));
    }
    /// @copydoc expect
    const T&& expect(const std::string& msg) const && {
        return std::visit(
            [&]<typename T0>(const T0&& arg) -> const T&& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return std::move(arg.value);
                } else if constexpr (std::is_same_v<std::decay_t<T0>, Err<Error>>) {
                    throw std::runtime_error(msg + ": " + arg.error.message);
                } else {
                    throw std::runtime_error(msg);
                }
            }, std::move(data_));
    }

    /**
     * @brief Returns the contained `Ok` value.
     * @return A reference to the contained `Ok` value.
     * @throw std::runtime_error if the `Result` is an `Err`.
     */
    T& unwrap() & {
        return expect("Attempted to unwrap error result");
    }
    /// @copydoc unwrap
    const T& unwrap() const & {
        return expect("Attempted to unwrap error result");
    }
    /// @copydoc unwrap
    T&& unwrap() && {
        return std::move(expect("Attempted to unwrap error result"));
    }
    /// @copydoc unwrap
    const T&& unwrap() const && {
        return std::move(expect("Attempted to unwrap error result"));
    }

    /**
     * @brief Returns the contained `Err` value.
     * @return A reference to the contained `Err` value.
     * @throw std::runtime_error if the `Result` is an `Ok`.
     */
    [[nodiscard]] E& unwrap_err() & {
        return std::visit(
            [&]<typename T0>(T0& arg) -> E& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Err<E>>) {
                    return arg.error;
                } else {
                  throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        data_);
    }
    /// @copydoc unwrap_err
    [[nodiscard]] const E& unwrap_err() const & {
        return std::visit(
            [&]<typename T0>(const T0& arg) -> const E& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Err<E>>) {
                    return arg.error;
                } else {
                  throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        data_);
    }
    /// @copydoc unwrap_err
    [[nodiscard]] E&& unwrap_err() && {
        return std::visit(
            [&]<typename T0>(T0&& arg) -> E&& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Err<E>>) {
                    return std::move(arg.error);
                } else {
                  throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        std::move(data_));
    }
    /// @copydoc unwrap_err
    [[nodiscard]] const E&& unwrap_err() const && {
        return std::visit(
            [&]<typename T0>(const T0&& arg) -> const E&& {
                if constexpr (std::is_same_v<std::decay_t<T0>, Err<E>>) {
                    return std::move(arg.error);
                } else {
                  throw std::runtime_error("Attempted to unwrap_err on ok result");
                }
            },
        std::move(data_));
    }

    /**
     * @brief Returns the contained `Err` value, otherwise throws a `std::runtime_error` with a custom message.
     * @param msg The message to include in the exception.
     * @return A reference to the contained `Err` value.
     * @throw std::runtime_error if the `Result` is an `Ok`.
     */
    [[nodiscard]] E& expect_err(const std::string& msg) & {
        if (is_err()) return std::get<Err<E>>(data_).error;
        throw std::runtime_error(msg);
    }
    /// @copydoc expect_err
    [[nodiscard]] const E& expect_err(const std::string& msg) const & {
        if (is_err()) return std::get<Err<E>>(data_).error;
        throw std::runtime_error(msg);
    }
    /// @copydoc expect_err
    [[nodiscard]] E&& expect_err(const std::string& msg) && {
        if (is_err()) return std::move(std::get<Err<E>>(data_).error);
        throw std::runtime_error(msg);
    }
    /// @copydoc expect_err
    [[nodiscard]] const E&& expect_err(const std::string& msg) const && {
        if (is_err()) return std::move(std::get<Err<E>>(data_).error);
        throw std::runtime_error(msg);
    }

    /**
     * @brief Applies a function to the contained value, or a fallback function to the error.
     *
     * This function is used to handle both `Ok` and `Err` cases in a single call.
     * Both functions must return the same type.
     *
     * @tparam FOk The type of the function to apply to the `Ok` value.
     * @tparam FErr The type of the function to apply to the `Err` value.
     * @param ok_fn The function to call with the `Ok` value if the `Result` is `Ok`.
     * @param err_fn The function to call with the `Err` value if the `Result` is `Err`.
     * @return The value returned by either `ok_fn` or `err_fn`.
     */
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, T&> &&
                 std::invocable<FErr, E&> &&
                 std::is_same_v<std::invoke_result_t<FOk, T&>, std::invoke_result_t<FErr, E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    /// @copydoc match
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, const T&> &&
                 std::invocable<FErr, const E&> &&
                 std::is_same_v<std::invoke_result_t<FOk, const T&>, std::invoke_result_t<FErr, const E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    /// @copydoc match
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, T&&> &&
                 std::invocable<FErr, E&&> &&
                 std::is_same_v<std::invoke_result_t<FOk, T&&>, std::invoke_result_t<FErr, E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    /// @copydoc match
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, const T&&> &&
                 std::invocable<FErr, const E&&> &&
                 std::is_same_v<std::invoke_result_t<FOk, const T&&>, std::invoke_result_t<FErr, const E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }

    /**
     * @brief Returns the contained `Ok` value or a provided default.
     * @param default_val The default value to return if the `Result` is `Err`.
     * @return The contained `Ok` value or `default_val`.
     */
    [[nodiscard]] T unwrap_or(const T& default_val) const & noexcept {
        if(is_ok()) return std::get<Ok<T>>(data_).value;
        return default_val;
    }
    /// @copydoc unwrap_or
    [[nodiscard]] T unwrap_or(T&& default_val) && noexcept {
        if(is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::move(default_val);
    }

    /**
     * @brief Returns the contained `Ok` value or computes it from a function.
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return The contained `Ok` value or the value returned by `f`.
     */
    template<typename F> requires std::invocable<F, E&> && std::is_same_v<std::invoke_result_t<F, E&>, T>
    T unwrap_or_else(F&& f) & {
        if (is_ok()) return std::get<Ok<T>>(data_).value;
        return std::invoke(std::forward<F>(f), std::get<Err<E>>(data_).error);
    }
    /// @copydoc unwrap_or_else
    template<typename F> requires std::invocable<F, const E&> && std::is_same_v<std::invoke_result_t<F, const E&>, T>
    T unwrap_or_else(F&& f) const & {
        if (is_ok()) return std::get<Ok<T>>(data_).value;
        return std::invoke(std::forward<F>(f), std::get<Err<E>>(data_).error);
    }
    /// @copydoc unwrap_or_else
    template<typename F> requires std::invocable<F, E&&> && std::is_same_v<std::invoke_result_t<F, E&&>, T>
    T unwrap_or_else(F&& f) && {
        if (is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::invoke(std::forward<F>(f), std::move(std::get<Err<E>>(data_).error));
    }
    /// @copydoc unwrap_or_else
    template<typename F> requires std::invocable<F, const E&&> && std::is_same_v<std::invoke_result_t<F, const E&&>, T>
    T unwrap_or_else(F&& f) const && {
        if (is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::invoke(std::forward<F>(f), std::move(std::get<Err<E>>(data_).error));
    }

    /**
     * @brief Calls a function with the `Ok` value if the `Result` is `Ok`.
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Ok` value.
     * @return A reference to the `Result`.
     */
    template<std::invocable<T&> F>
    Result& inspect(F&& f) & {
        if (is_ok()) {
            std::invoke(std::forward<F>(f), std::get<Ok<T>>(data_).value);
        }
        return *this;
    }
    /// @copydoc inspect
    template<std::invocable<const T&> F>
    const Result& inspect(F&& f) const & {
        if (is_ok()) {
            std::invoke(std::forward<F>(f), std::get<Ok<T>>(data_).value);
        }
        return *this;
    }
    /// @copydoc inspect
    template<std::invocable<T&&> F>
    Result&& inspect(F&& f) && {
        if (is_ok()) {
            std::invoke(std::forward<F>(f), std::move(std::get<Ok<T>>(data_).value));
        }
        return std::move(*this);
    }
    /// @copydoc inspect
    template<std::invocable<const T&&> F>
    const Result&& inspect(F&& f) const && {
        if (is_ok()) {
            std::invoke(std::forward<F>(f), std::move(std::get<Ok<T>>(data_).value));
        }
        return std::move(*this);
    }

    /**
     * @brief Calls a function with the `Err` value if the `Result` is `Err`.
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return A reference to the `Result`.
     */
    template<std::invocable<E&> F>
    Result& inspect_err(F&& f) & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<Err<E>>(data_).error);
        }
        return *this;
    }
    /// @copydoc inspect_err
    template<std::invocable<const E&> F>
    const Result& inspect_err(F&& f) const & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<Err<E>>(data_).error);
        }
        return *this;
    }
    /// @copydoc inspect_err
    template<std::invocable<E&&> F>
    Result&& inspect_err(F&& f) && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<Err<E>>(data_).error));
        }
        return std::move(*this);
    }
    /// @copydoc inspect_err
    template<std::invocable<const E&&> F>
    const Result&& inspect_err(F&& f) const && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<Err<E>>(data_).error));
        }
        return std::move(*this);
    }

    /**
     * @brief Returns a pointer to the contained value, or `nullptr`.
     * @return A pointer to the contained value if `Ok`, otherwise `nullptr`.
     */
    [[nodiscard]] T* try_unwrap() & noexcept {
        if(is_ok()) return &std::get<Ok<T>>(data_).value;
        return nullptr;
    }
    /// @copydoc try_unwrap
    [[nodiscard]] const T* try_unwrap() const & noexcept {
        if(is_ok()) return &std::get<Ok<T>>(data_).value;
        return nullptr;
    }

    /**
     * @brief Chains a computation that returns a `Result`.
     *
     * If the `Result` is `Ok`, calls `f` with the contained value and returns the `Result`.
     * If the `Result` is `Err`, returns the `Err` value.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Ok` value.
     * @return The `Result` returned by `f`, or the original `Err` value.
     */
    template<typename F> requires returns_result_for<F, T&>
    auto and_then(F&& f) & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    /// @copydoc and_then
    template<typename F> requires returns_result_for<F, const T&>
    auto and_then(F&& f) const & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    /// @copydoc and_then
    template<typename F> requires returns_result_for<F, T&&>
    auto and_then(F&& f) && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc and_then
    template<typename F> requires returns_result_for<F, const T&&>
    auto and_then(F&& f) const && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Maps a `Result<T, E>` to a `Result<U, E>` by applying a function to the `Ok` value.
     *
     * If the `Result` is `Ok`, calls `f` with the `Ok` value and returns a new `Result` with the result of `f`.
     * If the `Result` is `Err`, returns the `Err` value.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Ok` value.
     * @return A new `Result` with the value returned by `f`, or the original `Err` value.
     */
    template<std::invocable<T&> F>
    auto map(F&& f) & {
        return map_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map
    template<std::invocable<const T&> F>
    auto map(F&& f) const & {
        return map_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map
    template<std::invocable<T&&> F>
    auto map(F&& f) && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc map
    template<std::invocable<const T&&> F>
    auto map(F&& f) const && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Maps a `Result<T, E>` to a `Result<T, F>` by applying a function to the `Err` value.
     *
     * If the `Result` is `Err`, calls `f` with the `Err` value and returns a new `Result` with the result of `f`.
     * If the `Result` is `Ok`, returns the `Ok` value.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return A new `Result` with the error value returned by `f`, or the original `Ok` value.
     */
    template<std::invocable<E&> F>
    auto map_err(F&& f) & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map_err
    template<std::invocable<const E&> F>
    auto map_err(F&& f) const & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    /// @copydoc map_err
    template<std::invocable<E&&> F>
    auto map_err(F&& f) && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc map_err
    template<std::invocable<const E&&> F>
    auto map_err(F&& f) const && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Returns the `Result` if it is `Ok`, otherwise calls `f` with the `Err` value and returns the result.
     *
     * This is useful for error recovery.
     *
     * @tparam F The type of the function to call.
     * @param f The function to call with the `Err` value.
     * @return The original `Result` if `Ok`, or the `Result` returned by `f`.
     */
    template<typename F> requires returns_result_for<F, E&>
    Result<T, E> or_else(F&& f) & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    /// @copydoc or_else
    template<typename F> requires returns_result_for<F, const E&>
    Result<T, E> or_else(F&& f) const & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    /// @copydoc or_else
    template<typename F> requires returns_result_for<F, E&&>
    Result<T, E> or_else(F&& f) && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }
    /// @copydoc or_else
    template<typename F> requires returns_result_for<F, const E&&>
    Result<T, E> or_else(F&& f) const && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }

    /**
     * @brief Checks if the `Result` is `Ok` and contains the given value.
     * @tparam U The type of the value to compare against.
     * @param value The value to compare against.
     * @return `true` if the `Result` is `Ok` and the contained value is equal to `value`, `false` otherwise.
     */
    template<typename U>
    bool contains(const U& value) const noexcept {
        if(is_ok()) return std::get<Ok<T>>(data_).value == value;
        return false;
    }

    /**
     * @brief Converts the `Result` to a `std::optional<T>`.
     * @return An optional containing the `Ok` value if the `Result` is `Ok`, otherwise `std::nullopt`.
     */
    [[nodiscard]] std::optional<T> to_optional() const & noexcept {
        if(is_ok()) return std::get<Ok<T>>(data_).value;
        return std::nullopt;
    }
    /// @copydoc to_optional
    [[nodiscard]] std::optional<T> to_optional() && noexcept {
        if(is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::nullopt;
    }

private:
    /// @brief Private constructor for Ok results.
    template<typename... Args>
    explicit Result(std::in_place_index_t<0>, Args&&... args) : data_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    /// @brief Private constructor for Err results.
    template<typename... Args>
    explicit Result(std::in_place_index_t<1>, Args&&... args) : data_(std::in_place_index<1>, std::forward<Args>(args)...) {}

    /// @brief The underlying variant that holds either an `Ok<T>` or an `Err<E>`.
    std::variant<Ok<T>, Err<E>> data_;

    /// @brief Implementation of the `match` method.
    /// @brief Implementation of the `match` method.
    template <typename Self, typename FOk, typename FErr>
    static auto match_impl(Self&& self, FOk&& ok_fn, FErr&& err_fn) {
        using ResultType = std::invoke_result_t<FOk, decltype(std::forward<Self>(self).unwrap())>;

        return std::visit(
            [&]<typename T0>(T0&& arg) -> ResultType {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return std::invoke(std::forward<FOk>(ok_fn), std::forward<T0>(arg).value);
                } else {
                    return std::invoke(std::forward<FErr>(err_fn), std::forward<T0>(arg).error);
                }
            }, std::forward<Self>(self).data_);
    }

    /// @brief Implementation of the `map` method.
    template <typename Self, typename F>
    static auto map_impl(Self&& self, F&& f) {
        using ResultType = std::invoke_result_t<F, decltype(std::forward<Self>(self).unwrap())>;

        if constexpr (std::is_void_v<ResultType>) {
            return std::visit(
                [&]<typename T0>(T0&& arg) -> Result<void, E> {
                    if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                        std::invoke(std::forward<F>(f), std::forward<T0>(arg).value);
                        return Result<void, E>::ok();
                    } else {
                        return Result<void, E>::err(std::forward<T0>(arg).error);
                    }
                }, std::forward<Self>(self).data_);
        } else {
            return std::visit(
                [&]<typename T0>(T0&& arg) -> Result<ResultType, E> {
                    if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                        return Result<ResultType, E>::ok(
                            std::invoke(std::forward<F>(f), std::forward<T0>(arg).value)
                        );
                    } else {
                        return Result<ResultType, E>::err(std::forward<T0>(arg).error);
                    }
                }, std::forward<Self>(self).data_);
        }
    }

    /// @brief Implementation of the `and_then` method.
    template <typename Self, typename F>
    static auto and_then_impl(Self&& self, F&& f) {
        using ResultType = std::invoke_result_t<F, decltype(std::forward<Self>(self).unwrap())>;

        return std::visit(
            [&]<typename T0>(T0&& arg) -> ResultType {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return std::invoke(std::forward<F>(f), std::forward<T0>(arg).value);
                } else {
                    return ResultType::err(std::forward<T0>(arg).error);
                }
            }, std::forward<Self>(self).data_);
    }

    /// @brief Implementation of the `map_err` method.
    template <typename Self, typename F>
    static auto map_err_impl(Self&& self, F&& f) {
        using ErrorType = std::invoke_result_t<F, decltype(std::forward<Self>(self).unwrap_err())>;

        return std::visit(
            [&]<typename T0>(T0&& arg) -> Result<T, ErrorType> {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return Result<T, ErrorType>::ok(std::forward<T0>(arg).value);
                } else {
                    using FResult = std::invoke_result_t<F, decltype(std::forward<T0>(arg).error)>;
                    if constexpr (std::is_same_v<FResult, Err<ErrorType>>) {
                        return Result<T, ErrorType>::err(std::invoke(std::forward<F>(f), std::forward<T0>(arg).error));
                    } else {
                        return Result<T, ErrorType>::err_in_place(std::invoke(std::forward<F>(f), std::forward<T0>(arg).error));
                    }
                }
            }, std::forward<Self>(self).data_);
    }

    /// @brief Implementation of the `or_else` method.
    template <typename Self, typename F>
    static Result<T, E> or_else_impl(Self&& self, F&& f) {
        return std::visit(
            [&]<typename T0>(T0&& arg) -> Result<T, E> {
                if constexpr (std::is_same_v<std::decay_t<T0>, Ok<T>>) {
                    return Result<T, E>::ok(std::forward<T0>(arg).value);
                } else {
                    return std::invoke(std::forward<F>(f), std::forward<T0>(arg).error);
                }
            }, std::forward<Self>(self).data_);
    }

    template <typename, typename>
    friend class Result;
};

#endif // C++20