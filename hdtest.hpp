#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <sstream>

#if __has_include(<source_location>)
#   include <source_location>
    using sl = std::source_location;
#elif __has_include(<experimental/source_location>)
#   include <experimental/source_location>
    using sl = std::experimental::source_location;
#else
#   error "source_location is not available"
#endif

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"


struct TestContext {
    int failures = 0;
};

inline TestContext* G_current_test = nullptr;

struct TestAbort final {};

inline void report_failure(
    std::string_view what,
    std::string_view expr,
    std::string_view msg,
    const sl& loc = sl::current())
{
    G_current_test->failures++;

    std::cerr << "[ FAILED ] " << what
              << " at " << loc.file_name() << ":" << loc.line()
              << "\n expr: " << expr
              << "\n info: " << msg
              << "\n";
}

template <typename T, typename U>
inline void expect_eq(T lhs, U rhs, std::string_view expr, const sl& loc = sl::current()) {
    if (!(lhs == rhs)) {
        std::ostringstream message;
        message << "lhs != rhs (" << lhs << " != " << rhs << ")";
        report_failure("EXPECT_EQ", expr, message.str(), loc);
    }
}

#define EXPECT_EQ(lhs, rhs) expect_eq((lhs), (rhs), #lhs " == " #rhs);

template <typename T, typename U>
inline void expect_ne(T lhs, U rhs, std::string_view expr, const sl& loc = sl::current()) {
    if (!(lhs != rhs)) {
        std::ostringstream message;
        message << "lhs == rhs (" << lhs << " == " << rhs << ")";
        report_failure("EXPECT_NE", expr, message.str(), loc);
    }
}

#define EXPECT_NE(lhs, rhs) expect_ne((lhs), (rhs), #lhs " != " #rhs);

inline void expect_true(bool cond, std::string_view expr, const sl& loc = sl::current()) {
    if (!cond) {
        report_failure("EXPECT_TRUE", expr, "condition is false", loc);
    }
}

#define EXPECT_TRUE(cond) expect_true((cond), #cond);

inline void expect_false(bool cond, std::string_view expr, const sl& loc = sl::current()) {
    if (cond) {
        report_failure("EXPECT_FALSE", expr, "condition is true", loc);
    }
}

#define EXPECT_FALSE(cond) expect_false((cond), #cond);

template <typename T, typename U>
inline void assert_eq(T lhs, U rhs, std::string_view expr, const sl& loc = sl::current()) {
    if (!(lhs == rhs)) {
        std::istringstream message = "lhs != rhs (" << lhs << " != " << rhs << ")";
        report_failure("ASSERT_EQ", expr, message.str(), loc);
        throw TestAbort();
    }
}

#define ASSERT_EQ(lhs, rhs) assert_eq((lhs), (rhs), #lhs " == " #rhs);

template <typename T, typename U>
inline void assert_ne(T lhs, U rhs, std::string_view expr, const sl& loc = sl::current()) {
    if (!(lhs != rhs)) {
        std::istringstream message = "lhs == rhs (" << lhs << " == " << rhs << ")";
        report_failure("ASSERT_NE", expr, message.str(), loc);
        throw TestAbort();
    }
}

#define ASSERT_NE(lhs, rhs) assert_ne((lhs), (rhs), #lhs " != " #rhs);

inline void assert_true(bool cond, std::string_view expr, const sl& loc = sl::current()) {
    if (!cond) {
        report_failure("ASSERT_TRUE", expr, "condition is false", loc);
        throw TestAbort();
    }
}

#define ASSERT_TRUE(cond) assert_true((cond), #cond);

inline void assert_false(bool cond, std::string_view expr, const sl& loc = sl::current()) {
    if (cond) {
        report_failure("ASSERT_FALSE", expr, "condition is true", loc);
        throw TestAbort();
    }
}

#define ASSERT_FALSE(cond) assert_false((cond), #cond);


struct Test {
    Test(const std::string& name, std::function<void()> func)
        : name(name), func(func) {}

    std::string name;
    std::function<void()> func;
};

inline std::vector<Test>& GetTests() {
    static std::vector<Test> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> func) {
        GetTests().emplace_back(name, func);
    }
};

struct TestRunner {
    int runAllTests() {
        auto& tests = GetTests();
        int total_failures = 0;
        for (auto& test : tests) {
            TestContext ctx;
            G_current_test = &ctx;
            try {
                std::cout << "[        ] " << test.name << "\n";
                test.func();
            } catch (const TestAbort&) {

            }

            if (ctx.failures == 0) {
                std::cout << KGRN << "[ PASSED ] " << KNRM << test.name << "\n";
            } else {
                std::cout << KRED << "[ FAILED ] " << KNRM << test.name << "\n";
                total_failures++;
            }

        }

        if (total_failures > 0) {
            std::cout << "\nPassed: " << tests.size() - total_failures << "/" << tests.size()
                      << "\nFailed: " << total_failures << "\n";
        } else {
            std::cout << KGRN << "\nAll tests passed." << KNRM << "\n";
        }

        return total_failures;
    }
};


#define TEST(test_name) \
    void test_name();   \
    static TestRegistrar register_##test_name( #test_name, test_name); \
    void test_name()


#define MAIN_RUN_TESTS()                    \
    int main() {                            \
        TestRunner testRunner;              \
        return testRunner.runAllTests();    \
    }
