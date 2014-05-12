#include <string>
#include <vector>

#include "beanstalkpp.h"

using std::string;
using std::vector;

int main(int argc, char *argv[])
{
  vector<string> serverInfo = {"beater1:11300", "beater2:11300"};
  Beanstalkpp::Pool conns(serverInfo);
  std::cout << "##########--Test Start--###################" << std::endl;
  std::cout << "watching xz1: " << conns.watch("xz1") << std::endl;
  std::cout << "watching xz2: " << conns.watch("xz2") << std::endl;
  std::cout << "##########--After watch xz1 & xz2--########" << std::endl;
  for(auto & tube : conns.watching()) {
    std::cout << "watching tube: " << tube << std::endl;
  }
  std::cout << "###########################################" << std::endl;
  conns.use("gq");
  conns.bind("gq1", "gq");
  conns.bind("gq2", "gq");
  std::cout << "######--After bind gq1 & gq2 to gq--#######" << std::endl;
  for(auto & tube : conns.watching()) {
    std::cout << "watching tube: " << tube << std::endl;
  }
  std::cout << "#####--After unbind gq1 & gq2 with gq--####" << std::endl;
  conns.unbind("gq1", "gq");
  conns.unbind("gq2", "gq");
  for(auto & tube : conns.watching()) {
    std::cout << "watching tube: " << tube << std::endl;
  }
  std::cout << "#################--stats--#################" << std::endl;
  for(auto & stat : conns.stats()) {
   std::cout << stat << std::endl;
  }
  std::cout << "############--stat of tube xz--############" << std::endl;
  std::cout << conns.statsTube("xz1") << std::endl;
  return 0;
}
