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
    bst.use("xz");
    bst.use("gq");
    std::cout << bst.usIng() << std::endl;

    Beanstalkpp::Job job = bst.reserve();
    std::cout << "get msg: " << job.asString() << std::endl;
    bst.put("kitty");
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
