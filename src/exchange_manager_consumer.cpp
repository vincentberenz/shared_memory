#include "shared_memory/exchange_manager_consumer.hpp"

namespace shared_memory {


  Exchange_manager_consumer::Exchange_manager_consumer(std::string segment_id,
						       std::string object_id,
						       int max_exhange_size,
						       int serialization_size){

    segment_id_ = segment_id;
    object_id_consumer_ = object_id+"_consumer";
    object_id_producer_ = object_id+"_producer";
    serialization_size_ = serialization_size_;
    max_size_ = max_exchange_size*serialization_size+1;
    data_  = new double[max_size_+2];
    max_exchange_size_ = max_exchange_size;
    previous_producer_id_=-1;
    consumer_id_=-1;
    index_ = 2;
    
  }

    
  Exchange_manager_consumer::~Exchange_manager_consumer(){
    delete[] this->data_;
  }


  bool Exchange_manager_consumer::read_memory(){
    shared_memory.get(segment_id_,object_id_consumer_,data_,max_size_);
    int id = data_[0];
    nb_elements_  = data_[1];
    nb_consumed_ = 0;
    if (id==previous_id_){
      nb_elements_ = 0;
      index_ = -1;
      return false;
    }
    previous_id_ = id;
    index_=0;
    if(nb_elements_==0) {
      return false;
    }
    return true;
  }
    
  bool Exchange_manager_consumer::empty(){
    return (nb_consumed_==nb_elements_);
  }

  void Exchange_manager_consumer::consume(Serializable &serializable){
    serializable.create(data_+sizeof(double)*nb_consumed_*serialization_size);
    nb_consumed_++;
  }

  void Exchange_manager_consumer::write_to_memory(){
    static int to_producer[2];
    consumer_id_++;
    to_producer[0]=consumer_id_;
    to_producer[1]=nb_consumed_;
    shared_memory::set(segment_id_,object_id_consumer_,to_producer,2);
  }


}
