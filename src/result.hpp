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

struct Error {
    std::string message;
    int code = 0;

    bool operator==(const Error& other) const {
        return message == other.message && code == other.code;
    }
};

template <typename T>
struct Ok {
    T value;

    template<typename... Args>
        requires std::constructible_from<T, Args...>
    explicit(sizeof...(Args) == 1) Ok(Args&&... args)
        : value(std::forward<Args>(args)...) {}
};

template <typename E>
struct Err {
    E error;

    template<typename... Args>
        requires std::constructible_from<E, Args...>
    explicit(sizeof...(Args) == 1) Err(Args&&... args)
        : error(std::forward<Args>(args)...) {}
};

template<typename T, typename E = Error>
class Result;
template<typename E>
class Result<void, E>;

template<typename>
struct is_result : std::false_type {};
template<typename T, typename E>
struct is_result<Result<T, E>> : std::true_type {};
template<typename T>
inline constexpr bool is_result_v = is_result<T>::value;

template<typename F, typename... Args>
concept returns_result_for = std::invocable<F, Args...> && is_result_v<std::invoke_result_t<F, Args...>>;

// Specialization for Result<void>
template<typename E>
class Result<void, E> {
public:
    using value_type = void;
    using error_type = E;

    [[nodiscard]] static Result<void, E> ok() {
        return Result<void, E>(std::in_place_index<0>);
    }

    [[nodiscard]] static Result<void, E> err(E error) {
        return Result<void, E>(std::in_place_index<1>, std::move(error));
    }

    [[nodiscard]] static Result<void, E> err(const std::string& message, const int code = 0)
        requires std::is_same_v<E, Error>
    {
        return Result<void, E>(std::in_place_index<1>, Error{message, code});
    }

    template<typename... EArgs>
        requires std::constructible_from<E, EArgs...>
    [[nodiscard]] static Result<void, E> err_in_place(EArgs&&... args) {
        return Result<void, E>(std::in_place_index<1>, std::forward<EArgs>(args)...);
    }

    [[nodiscard]] bool is_ok() const noexcept { return std::holds_alternative<std::monostate>(data_); }
    [[nodiscard]] bool is_err() const noexcept { return std::holds_alternative<E>(data_); }
    explicit operator bool() const noexcept { return is_ok(); }


    void expect(const std::string& msg) & {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }
    void expect(const std::string& msg) const & {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }
    void expect(const std::string& msg) && {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }
    void expect(const std::string& msg) const && {
        if (is_err()) {
            if constexpr (std::is_same_v<E, Error>) {
                throw std::runtime_error(msg + ": " + std::get<E>(data_).message);
            } else {
                throw std::runtime_error(msg);
            }
        }
    }

    void unwrap() & {
        expect("Attempted to unwrap error result");
    }
    void unwrap() const & {
        expect("Attempted to unwrap error result");
    }
    void unwrap() && {
        std::move(*this).expect("Attempted to unwrap error result");
    }
    void unwrap() const && {
        std::move(*this).expect("Attempted to unwrap error result");
    }

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

    [[nodiscard]] E& expect_err(const std::string& msg) & {
        if (is_err()) return std::get<E>(data_);
        throw std::runtime_error(msg);
    }
    [[nodiscard]] const E& expect_err(const std::string& msg) const & {
        if (is_err()) return std::get<E>(data_);
        throw std::runtime_error(msg);
    }
    [[nodiscard]] E&& expect_err(const std::string& msg) && {
        if (is_err()) return std::move(std::get<E>(data_));
        throw std::runtime_error(msg);
    }
    [[nodiscard]] const E&& expect_err(const std::string& msg) const && {
        if (is_err()) return std::move(std::get<E>(data_));
        throw std::runtime_error(msg);
    }

    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, E&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, const E&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, const E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, E&&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    template<typename FOk, typename FErr>
      requires std::invocable<FOk> &&
          std::invocable<FErr, const E&&> &&
          std::is_same_v<std::invoke_result_t<FOk>, std::invoke_result_t<FErr, const E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }

    template<typename F> requires std::invocable<F, E&> && std::is_void_v<std::invoke_result_t<F, E&>>
    void unwrap_or_else(F&& f) & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<E>(data_));
        }
    }
    template<typename F> requires std::invocable<F, const E&> && std::is_void_v<std::invoke_result_t<F, const E&>>
    void unwrap_or_else(F&& f) const & {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::get<E>(data_));
        }
    }
    template<typename F> requires std::invocable<F, E&&> && std::is_void_v<std::invoke_result_t<F, E&&>>
    void unwrap_or_else(F&& f) && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<E>(data_)));
        }
    }
    template<typename F> requires std::invocable<F, const E&&> && std::is_void_v<std::invoke_result_t<F, const E&&>>
    void unwrap_or_else(std::function<void(E&&)> f) && {
        if (is_err()) {
            std::invoke(std::forward<F>(f), std::move(std::get<E>(data_)));
        }
    }

    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) const & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F>
    auto and_then(F&& f) const && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }

    template<typename F> requires std::invocable<F>
    auto map(F&& f) & {
        return map_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires std::invocable<F>
    auto map(F&& f) const & {
        return map_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires std::invocable<F>
    auto map(F&& f) && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }
    template<typename F> requires std::invocable<F>
    auto map(F&& f) const && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }

    template<typename F> requires std::invocable<F, E&>
    auto map_err(F&& f) & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires std::invocable<F, const E&>
    auto map_err(F&& f) const & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires std::invocable<F, E&&>
    auto map_err(F&& f) && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }
    template<typename F> requires std::invocable<F, const E&&>
    auto map_err(F&& f) const && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }

    template<typename F> requires returns_result_for<F, E&>
    Result<void, E> or_else(F&& f) & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, const E&>
    Result<void, E> or_else(F&& f) const & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, E&&>
    Result<void, E> or_else(F&& f) && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, const E&&>
    Result<void, E> or_else(F&& f) const && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }

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
    explicit Result(std::in_place_index_t<0>) : data_(std::in_place_index<0>) {}

    template<typename... Args>
    explicit Result(std::in_place_index_t<1>, Args&&... args) : data_(std::in_place_index<1>, std::forward<Args>(args)...) {}

    std::variant<std::monostate, E> data_;

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
    using value_type = T;
    using error_type = E;

    [[nodiscard]] static Result<T, E> ok(T value) {
        return Result<T, E>(std::in_place_index<0>, Ok<T>{std::move(value)});
    }

    template<typename U>
        requires std::constructible_from<T, U>
    [[nodiscard]] static Result<T, E> ok(U&& value) {
        return Result<T, E>(std::in_place_index<0>, T(std::forward<U>(value)));
    }

    template<typename... Args>
        requires std::constructible_from<T, Args...>
    [[nodiscard]] static Result<T, E> ok_in_place(Args&&... args) {
      return Result<T, E>(std::in_place_index<0>, Ok<T>(std::forward<Args>(args)...));
    }

    [[nodiscard]] static Result<T, E> err(E error) {
        return Result<T, E>(std::in_place_index<1>, Err<E>{std::move(error)});
    }

    [[nodiscard]] static Result<T, E> err(const std::string &message, const int code = 0)
        requires std::is_same_v<E, Error>
    {
        return Result<T, E>(std::in_place_index<1>, Err<Error>(message, code));
    }

    template<typename... EArgs>
        requires std::constructible_from<E, EArgs...>
    [[nodiscard]] static Result<T, E> err_in_place(EArgs&&... args) {
      return Result<T, E>(std::in_place_index<1>, Err<E>(std::forward<EArgs>(args)...));
    }

    [[nodiscard]] bool is_ok() const { return std::holds_alternative<Ok<T>>(data_);  }
    [[nodiscard]] bool is_err() const { return std::holds_alternative<Err<E>>(data_); }
    explicit operator bool() const noexcept { return is_ok(); }

    T* operator->() & { return &unwrap(); }
    const T* operator->() const & { return &unwrap(); }

    T& operator*() & { return unwrap(); }
    const T& operator*() const & { return unwrap(); }
    T&& operator*() && { return std::move(unwrap()); }
    const T&& operator*() const && { return std::move(unwrap()); }

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

    T& unwrap() & {
        return expect("Attempted to unwrap error result");
    }
    const T& unwrap() const & {
        return expect("Attempted to unwrap error result");
    }
    T&& unwrap() && {
        return std::move(expect("Attempted to unwrap error result"));
    }
    const T&& unwrap() const && {
        return std::move(expect("Attempted to unwrap error result"));
    }

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

    [[nodiscard]] E& expect_err(const std::string& msg) & {
        if (is_err()) return std::get<Err<E>>(data_).error;
        throw std::runtime_error(msg);
    }
    [[nodiscard]] const E& expect_err(const std::string& msg) const & {
        if (is_err()) return std::get<Err<E>>(data_).error;
        throw std::runtime_error(msg);
    }
    [[nodiscard]] E&& expect_err(const std::string& msg) && {
        if (is_err()) return std::move(std::get<Err<E>>(data_).error);
        throw std::runtime_error(msg);
    }
    [[nodiscard]] const E&& expect_err(const std::string& msg) const && {
        if (is_err()) return std::move(std::get<Err<E>>(data_).error);
        throw std::runtime_error(msg);
    }

    template<typename FOk, typename FErr>
        requires std::invocable<FOk, T&> &&
                 std::invocable<FErr, E&> &&
                 std::is_same_v<std::invoke_result_t<FOk, T&>, std::invoke_result_t<FErr, E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, const T&> &&
                 std::invocable<FErr, const E&> &&
                 std::is_same_v<std::invoke_result_t<FOk, const T&>, std::invoke_result_t<FErr, const E&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const & {
        return match_impl(*this, std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, T&&> &&
                 std::invocable<FErr, E&&> &&
                 std::is_same_v<std::invoke_result_t<FOk, T&&>, std::invoke_result_t<FErr, E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }
    template<typename FOk, typename FErr>
        requires std::invocable<FOk, const T&&> &&
                 std::invocable<FErr, const E&&> &&
                 std::is_same_v<std::invoke_result_t<FOk, const T&&>, std::invoke_result_t<FErr, const E&&>>
    auto match(FOk&& ok_fn, FErr&& err_fn) const && {
        return match_impl(std::move(*this), std::forward<FOk>(ok_fn), std::forward<FErr>(err_fn));
    }

    [[nodiscard]] T unwrap_or(const T& default_val) const & {
        if(is_ok()) return std::get<Ok<T>>(data_).value;
        return default_val;
    }
    [[nodiscard]] T unwrap_or(T&& default_val) && {
        if(is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::move(default_val);
    }

    template<typename F> requires std::invocable<F, E&> && std::is_same_v<std::invoke_result_t<F, E&>, T>
    T unwrap_or_else(F&& f) & {
        if (is_ok()) return std::get<Ok<T>>(data_).value;
        return std::invoke(std::forward<F>(f), std::get<Err<E>>(data_).error);
    }
    template<typename F> requires std::invocable<F, const E&> && std::is_same_v<std::invoke_result_t<F, const E&>, T>
    T unwrap_or_else(F&& f) const & {
        if (is_ok()) return std::get<Ok<T>>(data_).value;
        return std::invoke(std::forward<F>(f), std::get<Err<E>>(data_).error);
    }
    template<typename F> requires std::invocable<F, E&&> && std::is_same_v<std::invoke_result_t<F, E&&>, T>
    T unwrap_or_else(F&& f) && {
        if (is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::invoke(std::forward<F>(f), std::move(std::get<Err<E>>(data_).error));
    }
    template<typename F> requires std::invocable<F, const E&&> && std::is_same_v<std::invoke_result_t<F, const E&&>, T>
    T unwrap_or_else(F&& f) const && {
        if (is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::invoke(std::forward<F>(f), std::move(std::get<Err<E>>(data_).error));
    }

    [[nodiscard]] T* try_unwrap() & noexcept {
        if(is_ok()) return &std::get<Ok<T>>(data_).value;
        return nullptr;
    }
    [[nodiscard]] const T* try_unwrap() const & noexcept {
        if(is_ok()) return &std::get<Ok<T>>(data_).value;
        return nullptr;
    }

    template<typename F> requires returns_result_for<F, T&>
    auto and_then(F&& f) & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, const T&>
    auto and_then(F&& f) const & {
        return and_then_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, T&&>
    auto and_then(F&& f) && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, const T&&>
    auto and_then(F&& f) const && {
        return and_then_impl(std::move(*this), std::forward<F>(f));
    }

    template<std::invocable<T&> F>
    auto map(F&& f) & {
        return map_impl(*this, std::forward<F>(f));
    }
    template<std::invocable<const T&> F>
    auto map(F&& f) const & {
        return map_impl(*this, std::forward<F>(f));
    }
    template<std::invocable<T&&> F>
    auto map(F&& f) && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }
    template<std::invocable<const T&&> F>
    auto map(F&& f) const && {
        return map_impl(std::move(*this), std::forward<F>(f));
    }

    // For now, the `map_err` transforms the error but keeps the same type E
    template<std::invocable<E&> F>
    auto map_err(F&& f) & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    template<std::invocable<const E&> F>
    auto map_err(F&& f) const & {
        return map_err_impl(*this, std::forward<F>(f));
    }
    template<std::invocable<E&&> F>
    auto map_err(F&& f) && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }
    template<std::invocable<const E&&> F>
    auto map_err(F&& f) const && {
        return map_err_impl(std::move(*this), std::forward<F>(f));
    }

    template<typename F> requires returns_result_for<F, E&>
    Result<T, E> or_else(F&& f) & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, const E&>
    Result<T, E> or_else(F&& f) const & {
        return or_else_impl(*this, std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, E&&>
    Result<T, E> or_else(F&& f) && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }
    template<typename F> requires returns_result_for<F, const E&&>
    Result<T, E> or_else(F&& f) const && {
        return or_else_impl(std::move(*this), std::forward<F>(f));
    }

    template<typename U>
    bool contains(const U& value) const noexcept {
        if(is_ok()) return std::get<Ok<T>>(data_).value == value;
        return false;
    }

    [[nodiscard]] std::optional<T> to_optional() const & noexcept {
        if(is_ok()) return std::get<Ok<T>>(data_).value;
        return std::nullopt;
    }
    [[nodiscard]] std::optional<T> to_optional() && noexcept {
        if(is_ok()) return std::move(std::get<Ok<T>>(data_).value);
        return std::nullopt;
    }

private:
    template<typename... Args>
    explicit Result(std::in_place_index_t<0>, Args&&... args) : data_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    template<typename... Args>
    explicit Result(std::in_place_index_t<1>, Args&&... args) : data_(std::in_place_index<1>, std::forward<Args>(args)...) {}

    std::variant<Ok<T>, Err<E>> data_;

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