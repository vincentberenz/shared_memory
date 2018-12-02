#ifndef EXCHANGE_MANAGER_CONSUMER_HPP
#define EXCHANGE_MANAGER_CONSUMER_HPP

#include <string>
#include "shared_memory/shared_memory.hpp"
#include "shared_memory/serializable.hpp"

namespace shared_memory {


  template<class Serializable>
  class Exchange_manager_consumer {

  public:

    Exchange_manager_consumer(std::string segment_id,
			      std::string object_id,
			      int max_exhange_size);

    
    ~Exchange_manager_consumer();

    // read shared memory and returns true if there are is data to consume
    void read_memory();
    
    bool empty();

    void consume(Serializable &serializable);

    void write_to_memory();
    
    
    

  private:

    std::string segment_id_;
    std::string object_id_producer_;
    std::string object_id_consumer_;
    int max_exchange_size_;
    int id_;
    int consumer_id_;
    int previous_producer_id_;
    int max_size_;
    double *data_;
    int index_;
    int serialization_size_;
    int nb_elements_;
    int nb_consumed_;
    
  };

  
  #include "exchange_manager_consumer.hxx"
  

}




#endif // EXCHANGE_MANAGER_CONSUMER_HPP
