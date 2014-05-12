#include <string>
#include <vector>

#include "beanstalkpp.h"

using std::string;
using std::vector;

int main(int argc, char *argv[])
{
  vector<string> serverInfo = {"beater7:11300", "beater7:11301"};
  Beanstalkpp::Pool conns(serverInfo);
  conns.watch("xz");
  conns.use("gq");
  while(1) {
    Beanstalkpp::Job job = conns.reserve();
    int num = stoi(job.asString());
    if(num % 100 == 0) {
      conns.put("1");
    }
    job.del();
  }
  return 0;
}
