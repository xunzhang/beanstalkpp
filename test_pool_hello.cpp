#include <string>
#include <vector>
#include "beanstalkpp.h"

int main(int argc, char *argv[])
{
  {
    std::string info("beater7:11300");
    Beanstalkpp::Pool bst(info);
    std::cout << bst.watch("gq") << std::endl;
    bst.use("xz");
    std::cout << bst.usIng() << std::endl;

    bst.put("hello");
    Beanstalkpp::Job job = bst.reserve();
    std::cout << "get msg: " << job.asString() << std::endl;
  }
  std::cout << "------------------------------------------" << std::endl;

  /*  
  {
    std::vector<std::string> info = {"beater7:11301", "beater7:11302"};
    Beanstalkpp::Pool conns(info);
    conns.watch("xz");
    conns.watch("xz2");
    conns.watch("lol");
    for(auto & tube : conns.watching()) {
      std::cout << tube << std::endl;
    }

    conns.use("gq");
    conns.use("gq2");
    conns.use("hah");
    std::cout << conns.usIng() << std::endl;
  }
  */

  return 0;
}
