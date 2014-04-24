/**
 * Cluster Extension for Beanstalkpp
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef _BEANSTALK_CLUSTER_H
#define _BEANSTALK_CLUSTER_H

#include <string>
#include <vector>

#include "client.h"
#include "job.h"
#include "ring.h"
#include "utils.h"

namespace Beanstalkpp {

class Pool {

 private:
  void initOne(const std::string & s) {
    auto tmp = Beanstalkpp::str_split(s, ':');
    Beanstalkpp::Client *pt = new Beanstalkpp::Client(tmp[0], std::stoi(tmp[1]));
    pt->connect();
    this->conns.push_back(pt);
  }

  inline size_t tube_hash(const std::string & tube) {
    if(tube == "default") {
      return 0;
    }
    std::vector<size_t> ids;
    for(size_t i = 0; i < conns.size(); ++i) {
      ids.push_back(i);
    }
    Beanstalkpp::ring<size_t> tubeRing(ids);
    return tubeRing.get_server(tube);
  }

 // meta interface 
 public:
  Pool(std::string serverInfo) {
    initOne(serverInfo);
    watchingList.push_back("default");
    usedTube = "default";
  }

  Pool(std::vector<std::string> serversInfo) {
    for(auto & serverInfo : serversInfo) {
      initOne(serverInfo);
    }
    watchingList.push_back("default");
    usedTube = "default";
  }

  // TODO
  virtual ~Pool() {
    /*
    for(auto & conn : this->conns) {
      conn.close();
    }
    */
  }

  void use(const std::string & tube) {
    auto server_id = tube_hash(tube);
    conns[server_id]->use(tube);
    usedTube = tube;
  }

  std::string usIng() {
    return usedTube;
  }
  
  size_t watch(const std::string & tube) {
    auto server_id = tube_hash(tube);
    conns[server_id]->watch(tube);
    if(watchingList.find(tube) == watchingList.end()) {
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

  void bind(const std::string & tubeDst, 
            const std::string & tubeSrc) {}

  void bind(const std::vector<std::string> & tubesDst, 
            const std::vector<std::string> & tubesSrc) {}

  void unbind(const std::string & tubeDst, 
              const std::string & tubeSrc) {}

  void unbind(const std::vector<std::string> & tubesDst, 
              const std::vector<std::string> & tubesSrc) {}
 
 // communication interface
 public:
  int put(const std::string & msg) {
    auto server_id = tube_hash(usedTube);
    return conns[server_id]->put(msg);
  }

  // order do not guaranteed
  template <class TJob>
  TJob reserve() {
    for(auto & tube : watchingList) {
      auto server_id = tube_hash(tube);
      return conns[server_id]->reserve<TJob>();
    }
  }

  /*
  // another impl of reserve
  // order do not guaranteed
  template <class TJob>
  TJob reserve() {
    for(size_t i = 0; i < conns.size(); ++i) {
      return conns[i]->reserve<TJob>();
    }
  }
  */
  
  Job reserve() {
    return reserve<Job>();
  }

  template <class TJob>
  bool reserveWithTimeout(std::shared_ptr<TJob> & jobPtr, int timeout) {}

  bool reserveWithTimeout(job_p_t, int timeout) {}

  void delt(const Beanstalkpp::Job & j) {}

  void delt(const job_p_t & j) {}

  void bury(const Beanstalkpp::Job & j, int priority = 10) {}

 private:
  std::vector<Beanstalkpp::Client*> conns;
  // pool storage
  std::string usedTube;
  std::vector<std::string> watchingList;
}; 

} // namespace Beanstalkpp>
#endif
