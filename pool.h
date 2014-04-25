/**
 * Cluster Extension for Beanstalkpp
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef _BEANSTALK_CLUSTER_H
#define _BEANSTALK_CLUSTER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

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

  void break_rule(const std::string & tube, size_t sid) {
    bindMap[tube] = sid;
  }

  void recovery_rule(const std::string & tube) {
    bindMap.erase(tube);
  }

  inline size_t tube_hash(const std::string & tube) {
    if(tube == "default") {
      return 0;
    }
    if(bindMap.count(tube) == 1) {
      return bindMap[tube];
    }
    return tubeRingPtr->get_server(tube);
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
    std::vector<size_t> ids;
    for(size_t i = 0; i < conns.size(); ++i) {
      ids.push_back(i);
    }
    tubeRingPtr = new Beanstalkpp::ring<size_t>(ids);
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
    for(size_t i = 0; i < conns.size(); ++i) {
      delete conns[i];
    }
    delete tubeRingPtr;
  }

  void use(const std::string & tube) {
    auto serverId = tube_hash(tube);
    conns[serverId]->use(tube);
    usedTube = tube;
  }

  std::string usIng() {
    return usedTube;
  }
  
  size_t watch(const std::string & tube) {
    auto serverId = tube_hash(tube);
    conns[serverId]->watch(tube);
    if(std::find(watchingList.begin(), 
                 watchingList.end(), 
                 tube) != watchingList.end()) {
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
            const std::string & tubeSrc) {
    auto serverId = tube_hash(tubeSrc);
    break_rule(tubeDst, serverId);
    conns[serverId]->bind(tubeDst, tubeSrc);
  }

  void bind(const std::vector<std::string> & tubesDst, 
            const std::string & tubeSrc) {
    for(auto & tubeDst : tubesDst) {
      bind(tubeDst, tubeSrc);
    }
  }

  void unbind(const std::string & tubeDst, 
              const std::string & tubeSrc) {
    auto serverId = tube_hash(tubeSrc);
    recovery_rule(tubeDst);
    conns[serverId]->unbind(tubeDst, tubeSrc);
  }

  void unbind(const std::vector<std::string> & tubesDst, 
              const std::string & tubeSrc) {
    for(auto & tubeDst : tubesDst) {
      unbind(tubeDst, tubeSrc);
    }
  }
 
 // communication interface
 public:
  int put(const std::string & msg) {
    auto serverId = tube_hash(usedTube);
    return conns[serverId]->put(msg);
  }

  // order do not guaranteed
  template <class TJob>
  TJob reserve() {
    for(auto & tube : watchingList) {
      auto serverId = tube_hash(tube);
      return conns[serverId]->reserve<TJob>();
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
  bool reserveWithTimeout(std::shared_ptr<TJob> & jobPtr, 
                          int timeout) {
    for(auto & tube : watchingList) {
      auto serverId = tube_hash(tube);
      return conns[serverId]->reserveWithTimeout<TJob>(jobPtr, timeout);
    }
  }

  bool reserveWithTimeout(Beanstalkpp::job_p_t & jobPtr, int timeout) {
    return reserveWithTimeout(jobPtr, timeout);
  }

  void del(const Beanstalkpp::Job & j) {
    auto serverId = tube_hash(usedTube);
    return conns[serverId]->del(j);
  }

  void del(const Beanstalkpp::job_p_t & j) {
    auto serverId = tube_hash(usedTube);
    return conns[serverId]->del(j);
  }

  void bury(const Beanstalkpp::Job & j, int priority = 10) {
    auto serverId = tube_hash(usedTube);
    return conns[serverId]->bury(j, priority);
  }

 private:
  std::vector<Beanstalkpp::Client*> conns;
  Beanstalkpp::ring<size_t> *tubeRingPtr;
  // pool storage
  std::string usedTube;
  std::vector<std::string> watchingList;
  // little tricky: to break/recovery the tube_hash rule 
  std::unordered_map<std::string, size_t> bindMap;
}; 

} // namespace Beanstalkpp
#endif
