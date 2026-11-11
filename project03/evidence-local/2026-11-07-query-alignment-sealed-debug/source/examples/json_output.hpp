#pragma once
#include <ostream>
#include <string_view>
namespace p03::example {
inline void json_text(std::ostream& out,std::string_view value){
    constexpr char hex[]="0123456789abcdef";out<<'"';
    for(const char c:value){const auto b=static_cast<unsigned char>(c);
        if(c=='"' || c=='\\')out<<'\\'<<c;
        else if(b<32)out<<"\\u00"<<hex[b>>4]<<hex[b&15];else out<<c;
    }out<<'"';
}
template<class Range> void json_numbers(std::ostream& out,const Range& values){bool first=true;out<<'[';for(const auto v:values){if(!first)out<<',';first=false;out<<v;}out<<']';}
} // namespace p03::example
