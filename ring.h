/**
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef _BEANSTALKPP_RING_H 
#define _BEANSTALKPP_RING_H

#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // std::sort, std::find
#include <functional>
#include <unordered_map>

namespace Beanstalkpp {

// T rep type of server name
template <class T>
class ring {

public:
 
  ring(std::vector<T> names) {
    for(auto & name : names) {
      add_server(name);
    }
  }

  ring(std::vector<T> names, int cp) : replicas(cp) {
    for(auto & name : names) {
      add_server(name);
    }
  }

  void add_server(const T & name) {
    std::hash<std::string> hfunc;
    std::ostringstream tmp;
    tmp << name;
    auto name_str = tmp.str();
    for(int i = 0; i < replicas; ++i) {
      std::ostringstream cvt;
      cvt << i;
      auto n = name_str + ":" + cvt.str();
      auto key = hfunc(n);
      srv_hashring_dct[key] = name;
      srv_hashring.push_back(key);
    }
    // sort srv_hashring
    std::sort(srv_hashring.begin(), srv_hashring.end());
  }

  void remove_server(const T & name) {
    std::hash<std::string> hfunc;
    std::ostringstream tmp;
    tmp << name;
    auto name_str = tmp.str();
    for(int i = 0; i < replicas; ++i) {
      std::ostringstream cvt;
      cvt << i;
      auto n = name_str + ":" + cvt.str();
      auto key = hfunc(n);
      srv_hashring_dct.erase(key);
      auto iter = std::find(srv_hashring.begin(), srv_hashring.end(), key);
      if(iter != srv_hashring.end()) {
        srv_hashring.erase(iter);
      }
    } 
  }

  // TODO: relief load of srv_hashring_dct[srv_hashring[0]]
  template <class P>
  T get_server(const P & skey) {
    std::hash<P> hfunc;
    auto key = hfunc(skey);
    for(size_t i = 0; i < srv_hashring.size(); ++i) {
      auto server = srv_hashring[i];
      if(key <= server) {
        return srv_hashring_dct[server];
      }
    }
    return srv_hashring_dct[srv_hashring[0]];
  }

private:
  int replicas = 6;
  std::vector<size_t> srv_hashring;
  std::unordered_map<size_t, T> srv_hashring_dct;
};

} // namespace Beanstalkpp 

#endif
