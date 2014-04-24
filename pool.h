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
  void init(const std::string & s) {
    auto tmp = Beanstalkpp::str_split(s, ':');
    Beanstalkpp::Client *pt = new Beanstalkpp::Client(tmp[0], std::stoi(tmp[1]));
    pt->connect();
    this->conns.push_back(pt);
  }

  inline size_t tube_hash(const std::string & tube) {
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
    init(serverInfo);
  }

  Pool(std::vector<std::string> serversInfo) {
    for(auto & serverInfo : serversInfo) {
      init(serverInfo);
    }
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
  }
  
  size_t watch(const std::string & tube) {
    auto server_id = tube_hash(tube);
    size_t cnt = conns[server_id]->watch(tube);
    // cal watched tube cnt
    for(size_t i = 0; i < conns.size(); ++i) {
      if(i == server_id) continue;
      auto tubes = watching();
      cnt += tubes.size();
    }
    return cnt;
  }

  size_t watch(const std::vector<std::string> & tubes) {
    size_t cnt;
    for(auto & tube : tubes) {
      cnt = watch(tube);
    }
    return cnt;
  }

  // tubes contain conns.size() number of 'default'-tube
  std::vector<std::string> watching() {
    std::vector<std::string> tubes;
    for(size_t i = 0; i < conns.size(); ++i) {
      auto tmpTubes = conns[i]->watching();
      for(auto & tb : tmpTubes) {
        tubes.push_back(tb);
      }
    }
    return tubes;
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
  int put(const std::string & msg) {}

  template <class TJob>
  TJob reserve() {}

  template <class TJob>
  bool reserveWithTimeout(std::shared_ptr<TJob> & jobPtr, int timeout) {}

  bool reserveWithTimeout(job_p_t, int timeout) {}

  void delt(const Beanstalkpp::Job & j) {}

  void delt(const job_p_t & j) {}

  void bury(const Beanstalkpp::Job & j, int priority = 10) {}

  std::vector<std::string> listTubes() {}

 private:
  std::vector<Beanstalkpp::Client*> conns; // clients
}; 

} // namespace Beanstalkpp>
#endif
