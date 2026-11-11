#include "test.hpp"
#include <algorithm>
#include <iostream>

namespace test {
std::vector<Case>& registry() { static std::vector<Case> r; return r; }
std::size_t& assertions() { static std::size_t n=0; return n; }
}
int main(int argc,char** argv) {
    auto& cases=test::registry();
    std::sort(cases.begin(),cases.end(),[](const auto& a,const auto& b){return a.name<b.name;});
    std::string selected;
    if(argc==2 && std::string(argv[1])=="--list") {
        for(const auto& c:cases) std::cout<<c.name<<'\n';
        return 0;
    }
    if(argc==2 && std::string(argv[1])=="--self-check-failure") {
        try { CHECK(false); } catch(const std::exception& e) { std::cout<<"EXPECTED RUNNER FAILURE "<<e.what()<<'\n'; return 1; }
        return 0;
    }
    if(argc==3 && std::string(argv[1])=="--case") selected=argv[2];
    else if(argc!=1) { std::cerr<<"usage: p03_tests [--list | --case NAME | --self-check-failure]\n"; return 2; }
    std::size_t run=0,failed=0;
    for(const auto& c:cases) {
        if(!selected.empty() && selected!=c.name) continue;
        ++run; const auto before=test::assertions();
        try {
            c.run();
            if(test::assertions()==before) throw std::runtime_error("test has no assertions");
            std::cout<<"PASS "<<c.name<<" checks="<<test::assertions()-before<<'\n';
        } catch(const std::exception& e) {
            ++failed; std::cerr<<"FAIL "<<c.name<<" "<<e.what()<<'\n';
        } catch(...) { ++failed; std::cerr<<"FAIL "<<c.name<<" unknown exception\n"; }
    }
    if(run==0) { std::cerr<<"No tests selected\n"; return 2; }
    std::cout<<"RESULT cases="<<run<<" failed="<<failed<<" assertions="<<test::assertions()<<'\n';
    return failed==0 ? 0 : 1;
}
