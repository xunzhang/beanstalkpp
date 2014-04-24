#include <string>
#include <vector>
#include "beanstalkpp.h"

int main(int argc, char *argv[])
{
  {
    std::string info("beater7:11300");
    Beanstalkpp::Pool bst(info);
    std::cout << bst.watch("xz") << std::endl;
    auto tubes = bst.watching();
    for(auto & tube : tubes) {
      std::cout << tube << std::endl;
    }
    bst.use("gq");
  }

/*
  {
    std::vector<std::string> info = ["beater7:11300", "beater7:11477"];
    Beanstalkpp::Pool conns(info);
    conns.watch("xz");
    conn.use("gq");
  }
*/

  return 0;
}
