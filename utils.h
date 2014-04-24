#ifndef _BEANSTALKPP_UTILS_H
#define _BEANSTALKPP_UTILS_H

#include <string>
#include <vector>

namespace Beanstalkpp {

std::vector<std::string> 
str_split(const std::string & str, char sep) {
  size_t st = 0, en = 0;
  std::vector<std::string> r;
  while(1) {
    en = str.find(sep, st);
    std::string s = str.substr(st, en - st);
    if(s != "") r.push_back(s);
    if(en == std::string::npos) break;
    st = en + 1;
  }
  return r;
}

} // namespace Beanstalkpp

#endif
