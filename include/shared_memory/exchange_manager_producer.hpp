#ifndef EXCHANGE_MANAGER_PRODUCER_HPP
#define EXCHANGE_MANAGER_PRODUCER_HPP

#include <string>
#include <deque>
#include <stdexcept>
#include <cstring>	

#include "shared_memory/shared_memory.hpp"
#include "shared_memory/serializable.hpp"

namespace shared_memory {


  class Exchange_manager_producer {

  public:

    Exchange_manager_producer(std::string segment_id,
		     std::string object_id,
		     int max_exhange_size,
		     int serialization_size);

    
    ~Exchange_manager_producer();

    void set(const Serializable &serializable);

    void write_to_memory();

    void read_consumer_actions_and_update();

    void get_and_purge_consummed_ids(std::deque<int> &get);
    

  private:

    std::string segment_id_;
    std::string object_id_client_;
    std::string object_id_consumer_;
    int max_exchange_size_;
    int id_;
    int previous_consumer_id_;
    int max_size_;
    double *serialization_;
    double *data_;
    int index_;
    int serialization_size_;
    std::deque<int> consumed_;
    

  };


  
  
  

}




#endif // EXCHANGE_MANAGER_PRODUCER_HPP
