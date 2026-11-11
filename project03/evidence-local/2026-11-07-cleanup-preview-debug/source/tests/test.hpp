#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace test {
struct Case { std::string name; void (*run)(); };
std::vector<Case>& registry();
std::size_t& assertions();
struct Register { Register(const char* name,void(*f)()) { registry().push_back({name,f}); } };
inline void check(bool ok,const char* expr,const char* file,int line) {
    ++assertions();
    if(!ok) throw std::runtime_error(std::string(file)+":"+std::to_string(line)+": "+expr);
}
}
#define TEST_CASE(name) static void name(); static test::Register reg_##name(#name,name); static void name()
#define CHECK(...) test::check(static_cast<bool>((__VA_ARGS__)),#__VA_ARGS__,__FILE__,__LINE__)
#define CHECK_EQ(a,b) do { const auto check_a=(a); const auto check_b=(b); CHECK(check_a==check_b); } while(false)
#define CHECK_THROWS(expr, type) do { bool caught=false; try { (void)(expr); } catch(const type&) { caught=true; } CHECK(caught); } while(false)
