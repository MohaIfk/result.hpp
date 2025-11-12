#include <gtest/gtest.h>
#include <string>
#include <memory>       // For std::unique_ptr
#include <stdexcept>    // For std::runtime_error

#include "../src/result.hpp"

struct CustomError {
  int error_code;
  std::string details;

  bool operator==(const CustomError& other) const {
    return error_code == other.error_code && details == other.details;
  }
};

TEST(ResultTests, BasicConstruction) {
  // Result<T>
  auto ok_t = Result<int>::ok(42);
  EXPECT_TRUE(ok_t.is_ok());
  EXPECT_FALSE(ok_t.is_err());
  EXPECT_TRUE(ok_t);

  auto err_t = Result<int>::err("fail", 1);
  EXPECT_FALSE(err_t.is_ok());
  EXPECT_TRUE(err_t.is_err());
  EXPECT_FALSE(err_t);

  // Result<void>
  auto ok_v = Result<void>::ok();
  EXPECT_TRUE(ok_v.is_ok());
  EXPECT_FALSE(ok_v.is_err());
  EXPECT_TRUE(ok_v);

  auto err_v = Result<void>::err("fail", 2);
  EXPECT_FALSE(err_v.is_ok());
  EXPECT_TRUE(err_v.is_err());
  EXPECT_FALSE(err_v);
}

TEST(ResultTests, InPlaceFactories) {
  auto ok_t = Result<std::string>::ok_in_place(5, 'a'); // string("aaaaa")
  EXPECT_TRUE(ok_t.is_ok());
  EXPECT_EQ(ok_t.unwrap(), "aaaaa");

  auto err_t = Result<int>::err_in_place("bad error", 123);
  EXPECT_TRUE(err_t.is_err());
  EXPECT_EQ(err_t.unwrap_err().message, "bad error");
  EXPECT_EQ(err_t.unwrap_err().code, 123);
}

TEST(ResultTests, UnwrapAndExpectT) {
  Result<int> ok_res(Result<int>::ok(10));
  const Result<int> const_ok_res(Result<int>::ok(10));

  // Test & and const & overloads
  EXPECT_EQ(ok_res.unwrap(), 10);
  EXPECT_EQ(const_ok_res.unwrap(), 10);
  EXPECT_NO_THROW(ok_res.expect("should be ok"));

  // Test && overload
  EXPECT_EQ(Result<int>::ok(20).unwrap(), 20);

  // Test error cases
  Result<int> err_res(Result<int>::err("fail"));
  ASSERT_THROW(err_res.unwrap(), std::runtime_error);
  ASSERT_THROW(err_res.expect("custom"), std::runtime_error);

  // Test exception message
  try {
    err_res.expect("custom");
  } catch (const std::runtime_error& e) {
    std::string what = e.what();
    EXPECT_NE(what.find("custom: fail"), std::string::npos);
  }
}

TEST(ResultTests, UnwrapAndExpectVoid) {
  Result<void> ok_res(Result<void>::ok());
  Result<void> err_res(Result<void>::err("fail"));

  EXPECT_NO_THROW(ok_res.unwrap());
  EXPECT_NO_THROW(ok_res.expect("should be ok"));

  ASSERT_THROW(err_res.unwrap(), std::runtime_error);
  ASSERT_THROW(err_res.expect("custom"), std::runtime_error);

  try {
    err_res.expect("custom");
  } catch (const std::runtime_error& e) {
    std::string what = e.what();
    EXPECT_NE(what.find("custom: fail"), std::string::npos);
  }
}

TEST(ResultTests, UnwrapAndExpectErr) {
  Result<int> ok_res(Result<int>::ok(10));
  Result<int> err_res(Result<int>::err("fail", 99));
  const Result<int> const_err_res(Result<int>::err("const fail", 98));

  // Test & and const &
  EXPECT_EQ(err_res.unwrap_err().message, "fail");
  EXPECT_EQ(const_err_res.unwrap_err().code, 98);
  EXPECT_NO_THROW(err_res.expect_err("should be err"));

  // Test &&
  EXPECT_EQ(Result<int>::err("moved fail").unwrap_err().message, "moved fail");

  // Test error cases
  ASSERT_THROW(ok_res.unwrap_err(), std::runtime_error);
  ASSERT_THROW(ok_res.expect_err("custom"), std::runtime_error);

  // Test exception message
  try {
    ok_res.expect_err("custom");
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "custom");
  }
}

TEST(ResultTests, TryUnwrapAndToOptional) {
  Result<int> ok_res(Result<int>::ok(10));
  const Result<int> const_ok_res(Result<int>::ok(20));
  Result<int> err_res(Result<int>::err("fail"));

  // try_unwrap() -> T*
  int* val_ptr = ok_res.try_unwrap();
  ASSERT_NE(val_ptr, nullptr);
  EXPECT_EQ(*val_ptr, 10);
  *val_ptr = 11; // Test mutability
  EXPECT_EQ(ok_res.unwrap(), 11);

  // try_unwrap() -> const T*
  const int* const_val_ptr = const_ok_res.try_unwrap();
  ASSERT_NE(const_val_ptr, nullptr);
  EXPECT_EQ(*const_val_ptr, 20);

  // try_unwrap() on Err
  EXPECT_EQ(err_res.try_unwrap(), nullptr);

  // to_optional()
  EXPECT_EQ(ok_res.to_optional(), std::optional<int>(11));
  EXPECT_EQ(const_ok_res.to_optional(), std::optional<int>(20));
  EXPECT_EQ(err_res.to_optional(), std::nullopt);
  EXPECT_EQ(Result<int>::ok(50).to_optional(), std::optional<int>(50)); // Test && overload
}

TEST(ResultTests, UnwrapOr) {
  EXPECT_EQ(Result<int>::ok(10).unwrap_or(5), 10);
  EXPECT_EQ(Result<int>::err("fail").unwrap_or(5), 5);

  // Test && overload
  EXPECT_EQ(Result<int>::err("fail").unwrap_or(std::string("default").length()), 7);
}

TEST(ResultTests, UnwrapOrElse) {
  // Result<T>
  auto or_else_t = [](const Error& e) -> int { return e.message.length(); };
  EXPECT_EQ(Result<int>::ok(10).unwrap_or_else(or_else_t), 10);
  EXPECT_EQ(Result<int>::err("fail").unwrap_or_else(or_else_t), 4);

  // Result<void>
  bool called = false;
  auto or_else_v = [&](const Error& e) { called = true; };
  Result<void>::ok().unwrap_or_else(or_else_v);
  EXPECT_FALSE(called);

  Result<void>::err("fail").unwrap_or_else(or_else_v);
  EXPECT_TRUE(called);
}

TEST(ResultTests, Map) {
  auto to_string = [](int x) { return std::to_string(x); };
  auto to_void = [](int x) {};

  // T -> U (const &)
  const auto ok_t = Result<int>::ok(42);
  EXPECT_EQ(ok_t.map(to_string).unwrap(), "42");

  // T -> void
  EXPECT_TRUE(ok_t.map(to_void).is_ok());

  // Error propagation
  const auto err_t = Result<int>::err("fail");
  EXPECT_EQ(err_t.map(to_string).unwrap_err().message, "fail");

  // void -> U
  EXPECT_EQ(Result<void>::ok().map([]() { return 10; }).unwrap(), 10);

  // void -> void
  EXPECT_TRUE(Result<void>::ok().map([]() {}).is_ok());
}

TEST(ResultTests, AndThen) {
  auto to_str_res = [](int x) { return Result<std::string>::ok(std::to_string(x)); };
  auto to_err_res = [](int x) { return Result<std::string>::err("inner"); };

  // Success chain (const &)
  const auto ok_t = Result<int>::ok(42);
  EXPECT_EQ(ok_t.and_then(to_str_res).unwrap(), "42");

  // Failure chain
  EXPECT_EQ(ok_t.and_then(to_err_res).unwrap_err().message, "inner");

  // Error propagation (short-circuit)
  const auto err_t = Result<int>::err("outer");
  int calls = 0;
  auto counter_fn = [&](int x) { calls++; return Result<int>::ok(x); };
  EXPECT_EQ(err_t.and_then(counter_fn).unwrap_err().message, "outer");
  EXPECT_EQ(calls, 0); // Prove it was never called
}

TEST(ResultTests, OrElse) {
  auto recover = [](const Error& e) { return Result<int>::ok(e.message.length()); };
  auto fail_recover = [](const Error& e) { return Result<int>::err("still fail"); };

  // Ok pass-through (const &)
  const auto ok_t = Result<int>::ok(42);
  EXPECT_EQ(ok_t.or_else(recover).unwrap(), 42);

  // Error recovery
  const auto err_t = Result<int>::err("fail");
  EXPECT_EQ(err_t.or_else(recover).unwrap(), 4);

  // Chained error
  EXPECT_EQ(err_t.or_else(fail_recover).unwrap_err().message, "still fail");
}

TEST(ResultTests, MapErr) {
  auto remap = [](const Error& e) { return Error{e.message + "ed", 10}; };

  // Ok pass-through (const &)
  const auto ok_t = Result<int>::ok(42);
  EXPECT_EQ(ok_t.map_err(remap).unwrap(), 42);

  // Error remapping
  const auto err_t = Result<int>::err("fail");
  auto remap_res = err_t.map_err(remap);
  EXPECT_EQ(remap_res.unwrap_err().message, "failed");
  EXPECT_EQ(remap_res.unwrap_err().code, 10);
}

TEST(ResultTests, MoveOnlyTypeSupport) {
  auto factory = []() {
    return Result<std::unique_ptr<int>>::ok(std::make_unique<int>(10));
  };

  // Test unwrap() &&
  auto p = factory().unwrap(); // Moves the unique_ptr out
  EXPECT_EQ(*p, 10);

  // Test operator->
  auto res_ptr = factory();
  EXPECT_EQ(*(res_ptr.operator->()->get()), 10);

  // Test operator*
  EXPECT_EQ(**res_ptr, 10); // Calls ->unwrap().operator*()

  // Test map() && (move-only chain)
  auto map_res = factory().map([](std::unique_ptr<int> p_in) {
      *p_in += 5;
      return p_in; // p_in is moved out
  });
  EXPECT_TRUE(map_res.is_ok());
  EXPECT_EQ(**map_res, 15);

  // Test and_then() && (full move-only chain)
  auto final_res = factory()
      .and_then([](std::unique_ptr<int> p_in) {
          *p_in += 1; // *p_in is 11
          return Result<std::unique_ptr<std::string>>::ok(
              std::make_unique<std::string>(std::to_string(*p_in))
          );
      })
      .map([](std::unique_ptr<std::string> s_in) {
          return *s_in + "!"; // s_in is moved, string is copied
      });

  EXPECT_TRUE(final_res.is_ok());
  EXPECT_EQ(final_res.unwrap(), "11!");

  // Test move-only error propagation
  auto err_chain = factory()
      .and_then([](std::unique_ptr<int> p_in) {
          return Result<int>::err("chain fail");
      })
      .map([](int x) { return x + 1; });

  EXPECT_TRUE(err_chain.is_err());
  EXPECT_EQ(err_chain.unwrap_err().message, "chain fail");
}

TEST(ResultTests, ContainsMethod) {
  auto result = Result<int>::ok(42);
  EXPECT_TRUE(result.contains(42));
  EXPECT_FALSE(result.contains(0));
}

TEST(ResultTests, Match) {
  std::string ok_val;
  std::string err_val;

  auto ok_fn = [&](int& x) { ok_val = std::to_string(x); x = 100; return "ok"; };
  auto err_fn = [&](Error& e) { err_val = e.message; e.message = "handled"; return "err"; };

  // Test lvalue Ok
  Result<int> ok_res = Result<int>::ok(42);
  EXPECT_EQ(ok_res.match(ok_fn, err_fn), "ok");
  EXPECT_EQ(ok_val, "42");
  EXPECT_EQ(err_val, "");
  EXPECT_EQ(ok_res.unwrap(), 100); // Check mutation

  // Test lvalue Err
  ok_val.clear();
  Result<int> err_res = Result<int>::err("fail");
  EXPECT_EQ(err_res.match(ok_fn, err_fn), "err");
  EXPECT_EQ(ok_val, "");
  EXPECT_EQ(err_val, "fail");
  EXPECT_EQ(err_res.unwrap_err().message, "handled"); // Check mutation

  // Test const lvalue Ok
  const Result<int> const_ok_res = Result<int>::ok(10);
  auto const_ok_fn = [](const int& x) { return x * 2; };
  auto const_err_fn = [](const Error& e) { return (int)e.message.length(); };
  EXPECT_EQ(const_ok_res.match(const_ok_fn, const_err_fn), 20);

  // Test rvalue (moved) Ok
  EXPECT_EQ(Result<int>::ok(7).match(
      [](int&& x) { return std::to_string(std::move(x)); },
      [](Error&& e) { return e.message; }
  ), "7");

  // Test rvalue (moved) Err
  EXPECT_EQ(Result<int>::err("rvalue").match(
      [](int&& x) -> std::string { return std::to_string(std::move(x)); },
      [](Error&& e) -> std::string { return e.message; }
  ), "rvalue");

  // Test void specialization
  bool ok_called = false;
  bool err_called = false;
  Result<void>::ok().match(
      [&]() { ok_called = true; },
      [&](Error&& e) { err_called = true; }
  );
  EXPECT_TRUE(ok_called);
  EXPECT_FALSE(err_called);

  ok_called = false;
  err_called = false;
  Result<void>::err("void fail").match(
      [&]() { ok_called = true; },
      [&](Error&& e) { err_called = true; }
  );
  EXPECT_FALSE(ok_called);
  EXPECT_TRUE(err_called);
}

TEST(ResultTests, GeneralizedErrorTypeE) {
  // Use the full template, not the alias
  using CustomResult = Result<int, CustomError>;

  auto ok_res = CustomResult::ok(100);
  auto err_res = CustomResult::err(CustomError{404, "Not Found"});

  // Basic checks
  EXPECT_TRUE(ok_res.is_ok());
  EXPECT_FALSE(err_res.is_ok());
  EXPECT_EQ(ok_res.unwrap(), 100);
  EXPECT_EQ(err_res.unwrap_err().error_code, 404);

  // Test expect (which can't print custom error details)
  ASSERT_THROW(err_res.expect("custom"), std::runtime_error);
  try {
    err_res.expect("custom");
  } catch(const std::runtime_error& e) {
    // Note: expect() doesn't know about CustomError.message
    EXPECT_STREQ(e.what(), "custom");
  }

  // map
  auto map_res = err_res.map([](int x) { return x * 2; });
  EXPECT_TRUE(map_res.is_err());
  EXPECT_EQ(map_res.unwrap_err().details, "Not Found");

  // and_then
  auto and_then_res = ok_res.and_then([](int x) {
    return Result<std::string, CustomError>::err(CustomError{500, "Server Error"});
  });
  EXPECT_TRUE(and_then_res.is_err());
  EXPECT_EQ(and_then_res.unwrap_err().error_code, 500);

  // map_err
  auto map_err_res = err_res.map_err([](const CustomError& e) {
    return CustomError{e.error_code, e.details + " (mapped)"};
  });
  EXPECT_EQ(map_err_res.unwrap_err().details, "Not Found (mapped)");

  // or_else
  auto or_else_res = err_res.or_else([](const CustomError& e) {
    if (e.error_code == 404) {
        return CustomResult::ok(0); // Recover with a default
    }
    return CustomResult::err(e);
  });
  EXPECT_TRUE(or_else_res.is_ok());
  EXPECT_EQ(or_else_res.unwrap(), 0);

  // match
  auto match_res = err_res.match(
      [](int x) { return std::to_string(x); },
      [](const CustomError& e) { return e.details; }
  );
  EXPECT_EQ(match_res, "Not Found");
}

TEST(ResultTests, LvalueConstAccessors) {
  // Test operator* and operator-> on mutable lvalues
  Result<std::string> ok_res = Result<std::string>::ok("hello");
  EXPECT_EQ(*ok_res, "hello");
  EXPECT_EQ(ok_res->length(), 5);

  *ok_res = "world";
  EXPECT_EQ(ok_res.unwrap(), "world");

  // Test operator* and operator-> on const lvalues
  const Result<std::string> const_ok_res = Result<std::string>::ok("const");
  EXPECT_EQ(*const_ok_res, "const");
  EXPECT_EQ(const_ok_res->length(), 5);
  // *const_ok_res = "fail"; // This needs to fail to compile, I just don't know how to automate this test. TODO: each time I uncomment it then test the failing
}

TEST(ResultTests, TemplatedFactories) {
  // Test template<typename U> ok(U&& value)
  const char* c_str = "hello";
  auto ok_from_cstr = Result<std::string>::ok(c_str);
  EXPECT_EQ(ok_from_cstr.unwrap(), "hello");

  // Test [[nodiscard]] static Result<T, E> err(E error)
  // (using CustomError to distinguish from the (string, int) factory)
  auto err_from_custom = Result<int, CustomError>::err(CustomError{1, "test"});
  EXPECT_TRUE(err_from_custom.is_err());
  EXPECT_EQ(err_from_custom.unwrap_err().details, "test");
}

TEST(ResultTests, ConstRvalueOverloads) {
  // Create a const object
  const Result<int> const_ok = Result<int>::ok(10);

  // Move from the const object (invokes const&& overload)
  auto map_res = std::move(const_ok).map([](const int& x) { return x * 2; });
  EXPECT_EQ(map_res.unwrap(), 20);

  const Result<int> const_err = Result<int>::err("fail");
  auto err_res = std::move(const_err).map_err([](const Error& e) {
      return Error{e.message + "ed", e.code};
  });
  EXPECT_EQ(err_res.unwrap_err().message, "failed");

  // Test unwrap() const&&
  const Result<int> const_ok_2 = Result<int>::ok(50);
  EXPECT_EQ(std::move(const_ok_2).unwrap(), 50);
}