#include <bbp/promise.h>
#include <gtest/gtest.h>
#include <string>

TEST(bbp_promise, basic)
{
    bbp::promise<int> p([](auto fulfill, auto reject) { fulfill(3); });
}

TEST(bbp_promise, empty_promise)
{
    bbp::promise<> p([](auto fulfill, auto reject) { fulfill(); });
}

// Check that when the then function has a 'void' return, then the
// corresponding
// promise is an empty promise.
TEST(bbp_promise, void_then)
{
    bbp::promise<> foo = bbp::promise<>([](auto fulfill, auto reject) {
                             fulfill();
                         }).then([]() {});

    bbp::promise<> bar = bbp::promise<int>([](auto fulfill, auto reject) {
                             fulfill(3);
                         }).then([](int) {}, [](std::exception_ptr) {});
}

TEST(bbp_promise, then_two_arg)
{
    //  Note the error message here. Concepts doesn't help.
    //  bbp::promise<int> q([](auto fulfill, auto reject) { fulfill("33"); });
    bbp::promise<std::string> ps =
        bbp::promise<int>([](auto fulfill, auto reject) { fulfill(3); })
            .then([](int i) { return std::string(std::to_string(i)); },
                  [](std::exception_ptr) { return std::string("error"); });

    std::string result;
    ps.then(
        [&](auto s) {
            result = s;
            return bbp::monostate();
        },
        [&](auto e) {
            result = "error";
            return bbp::monostate();
        });
    EXPECT_EQ(result, "3") << "The then function wasn't called.";

    result = "";
    ps.then(
          [&](auto s) {
              throw std::runtime_error("error");
              return bbp::monostate();
          },
          [&](auto e) {
              result = "error";
              return bbp::monostate();
          })
        .then(
            [&](auto s) {
                result = "value";
                return bbp::monostate();
            },
            [&](std::exception_ptr e) {
                result = "expected_error";
                try {
                    std::rethrow_exception(e);
                }
                catch (const std::runtime_error& error) {
                    EXPECT_EQ(error.what(), std::string("error"));
                }
                catch (...) {
                    ADD_FAILURE() << "Unexpected exception thrown.";
                }
                return bbp::monostate();
            });
    EXPECT_EQ(result, "expected_error") << "Error handling didn't happen.";
}

TEST(bbp_promise, then_one_arg)
{
    std::string result;
    bbp::promise<int>([](auto fulfill, auto reject) {
        fulfill(3);
    }).then([](int i) {
          return std::string(std::to_string(i));
      }).then([&](auto s) {
        result = s;
        return bbp::monostate();
    });
    EXPECT_EQ(result, "3") << "The then function wasn't called.";

    result = "";
    bbp::promise<bbp::monostate>(
        [](auto fulfill, auto reject) { fulfill(bbp::monostate()); })
        .then([](bbp::monostate) -> bbp::monostate {
            throw std::runtime_error("exception");
        })
        .then(
            [&](auto s) {
                result = "value";
                return bbp::monostate();
            },
            [&](std::exception_ptr e) {
                result = "expected_error";
                try {
                    std::rethrow_exception(e);
                }
                catch (const std::runtime_error& error) {
                    EXPECT_EQ(error.what(), std::string("exception"));
                }
                catch (...) {
                    ADD_FAILURE() << "Unexpected exception thrown.";
                }
                return bbp::monostate();
            });
    EXPECT_EQ(result, "expected_error") << "Error handling didn't happen.";
}

TEST(bbp_promise, fulfill)
{
    bbp::promise<int, double> p = bbp::promise<>::fulfill(3, 2.5);

    bool fulfilled = false;
    p.then([&fulfilled](int i, double d) {
        fulfilled = true;
        EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
        EXPECT_EQ(d, 2.5) << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
}

TEST(bbp_promise, reject)
{
    std::exception_ptr error;
    try {
        throw std::runtime_error("test");
    }
    catch (...) {
        error = std::current_exception();
    }

    bbp::promise<int, double> p = bbp::promise<>::reject<int,double>(error);

    bool rejected = false;
    p.then([](int i, double d) { ADD_FAILURE() << "Unexpected fulfillment."; },
           [&rejected, &error](std::exception_ptr e) {
               EXPECT_EQ(e, error) << "Rejected with wrong exception";
               rejected = true;
           });
    EXPECT_TRUE(rejected) << "Promise wasn't rejected.";
}

// TODO: add some tests that interact with asio.
//       - ASIO_STANDALONE is the definition required.
// TODO: add test that exceptions in the resolver are rethrown.
// TODO: add test that in the two-function version the exception handler is
// called.
// TODO: Test that promise<std::exception_ptr> works properly.

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}