#include <string>
#include <vector>

#include "beanstalkpp.h"

using std::string;
using std::vector;

int main(int argc, char *argv[])
{
  vector<string> serverInfo = {"beater7:11300", "beater7:11301"};
  Beanstalkpp::Pool conns(serverInfo);
  conns.use("xz");
  conns.watch("gq");
  for(int i = 0; i < 10000; ++i) {
    conns.put(std::to_string(i));
  }
  int sum = 0;
  int cnt = 100;
  while(1) {
    Beanstalkpp::Job job = conns.reserve();
    sum += stoi(job.asString());
    job.del();
    std::cout << "result is: " << sum << std::endl;
  }
  return 0;
}
