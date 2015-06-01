/**
 * Pool of Connections 
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef _BEANSTALKPP_POOL_H
#define _BEANSTALKPP_POOL_H

#include <unistd.h>
#include <vector>
#include <algorithm>

#include "client.h"
#include "job.h"
#include "utils.h"

namespace Beanstalkpp {

class Pool {

 private:
  void initOne(const std::string & s) {
    auto tmp = Beanstalkpp::str_split(s, ':');
    Beanstalkpp::Client *pt = new Beanstalkpp::Client(
        tmp[0],
        std::stoi(tmp[1])
        );
    pt->connect();
    conns.push_back(pt);
  }

 // declare interface
 public:
  Pool(std::string serverInfo) {
    initOne(serverInfo);
    use("default");
    usedTube = "default";
    watch("default");
    watchingList.push_back("default");
  }

  Pool(std::vector<std::string> serversInfo) {
    for(auto & serverInfo : serversInfo) {
      initOne(serverInfo);
    }
    use("default");
    usedTube = "default";
    watch("default");
    watchingList.push_back("default");
  }

  virtual ~Pool() {
    size_t sz = conns.size();
    for(size_t i = 0; i < sz; ++i) {
      delete conns[i];
    }
  }

  Pool(const Pool & p) : usedTube(p.usedTube) {
    conns = p.conns;
    watchingList = p.watchingList;
  }

  Pool& operator = (const Pool & p) {
    usedTube = p.usedTube;
    conns = p.conns;
    watchingList = p.watchingList;
    return *this;
  }
  
  Pool(Pool && p) : usedTube(p.usedTube) {
    std::swap(conns, p.conns);
    std::swap(watchingList, p.watchingList);
  }

  Pool& operator = (Pool && p) {
    std::swap(usedTube, p.usedTube);
    std::swap(conns, p.conns);
    std::swap(watchingList, p.watchingList);
    return *this;
  }

  void use(const std::string & tube) {
    for(auto & conn : conns) {
      conn->use(tube);
    }
  }

  std::string usIng() {
    return usedTube;
  }

  size_t watch(const std::string & tube) {
    for(auto & conn : conns) {
      conn->watch(tube);
    }
    if(std::find(watchingList.begin(), 
                 watchingList.end(), 
                 tube) == watchingList.end()) {
      watchingList.push_back(tube);
    }
    return watchingList.size();
  }

  size_t watch(const std::vector<std::string> & tubes) {
    for(auto & tube : tubes) {
      watch(tube);
    }
    return watchingList.size();
  }

  std::vector<std::string> watching() {
    return watchingList;
  }

  // bind tubeDst to tubeSrc
  void bind(const std::string & tubeDst, 
            const std::string & tubeSrc) {
    for(auto & conn : conns) {
      conn->bind(tubeDst, tubeSrc);
    }
  }

  // bind tubesDst to tubeSrc
  void bind(const std::vector<std::string> & tubesDst, 
            const std::string & tubeSrc) {
    for(auto & tubeDst : tubesDst) {
      bind(tubeDst, tubeSrc);
    }
  }

  // unbind tubeDst with tubeSrc
  void unbind(const std::string & tubeDst, 
              const std::string & tubeSrc) {
    for(auto & conn : conns) {
      conn->unbind(tubeDst, tubeSrc);
    }
  }

  // unbind tubesDst with tubeSrc
  void unbind(const std::vector<std::string> & tubesDst, 
              const std::string & tubeSrc) {
    for(auto & tubeDst : tubesDst) {
      unbind(tubeDst, tubeSrc);
    }
  }

  std::vector<std::string> stats() {
    std::vector<std::string> r;
    for(auto & conn : conns) {
      try {
        r.push_back(conn->stats());
      } catch (...) {
        std::cout << "stats fail" << std::endl;
      }
    }
    return r;
  }

  std::string statsTube(const std::string & tube) {
    std::string r;
    for(auto & conn : conns) {
      try {
        r = conn->statsTube(tube);
        return r;
      } catch (...) {
        std::cout << "statsTube fail: " << tube << std::endl;
      }
    }
    return r;
  }

 // communication interface
 public:
  int put(const std::string & msg) {
    int r;
    for(auto & conn : conns) {
      try {
        return conn->put(msg);
      } catch (...) {
        std::cout << "put failed" << std::endl;
        updateConn();
      }
    } 
    return -1;
  }

  // order can not be guaranteed
  template <class TJob>
  TJob reserve() {
    while(1) {
      for(auto & conn : conns) {
        try {
          return conn->reserve<TJob>();
        } catch (...) {
          std::cout << "reserve failed" << std::endl;
          updateConn();
        }
      } // for
      usleep(100);
    } // while
  }

  Beanstalkpp::Job reserve() {
    return reserve<Beanstalkpp::Job>();
  }

  // at most wait [timeout * conns.size()] seconds
  // return value differ from reserve interface
  template <class TJob>
  bool reserveWithTimeout(boost::shared_ptr<TJob> & jobPtr,
                          int timeout) {
    for(auto & conn : conns) {
      try {
        bool r = conn->reserveWithTimeout<TJob>(jobPtr, timeout);
        if(r) {
          return true;
        }
      } catch (...) {
        std::cout << "reserveWithTimeout failed" << std::endl;
      }
    } // for
    return false;
  }

  bool reserveWithTimeout(Beanstalkpp::job_p_t & jobPtr,
                          int timeout) {
    return reserveWithTimeout<Beanstalkpp::Job>(jobPtr, timeout);
  }
 
 private:
  inline void updateConn() {
    std::vector<Beanstalkpp::Client*> tmp(conns.begin() + 1, conns.end());
    tmp.push_back(conns[0]);
    conns = tmp;
  }

 private:
  std::vector<Beanstalkpp::Client*> conns;
  std::string usedTube;
  std::vector<std::string> watchingList;
};

} // namespace Beanstalkpp

#endif
