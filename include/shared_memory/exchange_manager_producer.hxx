
template <class Serializable>
Exchange_manager_producer<Serializable>::Exchange_manager_producer(std::string segment_id,
								   std::string object_id,
								   int max_exchange_size)
  : items_(max_exchange_size) {

  segment_id_ = segment_id;
  object_id_producer_ = object_id+"_producer";
  object_id_consumer_ = object_id+"_consumer";
  
  previous_consumer_id_ = -1;

  // init of shared memory
  double foo[2];
  foo[0]=static_cast<double>(1);
  foo[1]=static_cast<double>(0);
  shared_memory::set(segment_id_,object_id_consumer_,foo,2);
  shared_memory::set(segment_id_,object_id_producer_,
		     items_.get_data(),items_.get_data_size());
  
}

template <class Serializable>  
Exchange_manager_producer<Serializable>::~Exchange_manager_producer(){}
  

template <class Serializable>
void Exchange_manager_producer<Serializable>::set(const Serializable &serializable){
  items_.add(serializable);
}


template <class Serializable>
void Exchange_manager_producer<Serializable>::update_memory(std::deque<int> &get_consumed_ids){

  // read shared memory to check if the consumer
  // did consume any data
  static double from_consumer[2];

  // format expected: [id, number of items consumed by consumer]
  shared_memory::get(segment_id_,object_id_consumer_,from_consumer,2);
  int consumer_id = static_cast<int>(from_consumer[0]);
  int nb_to_remove = static_cast<int>(from_consumer[1]);
  
  // if "id" is -1, the consumer did not consume anything since
  // last time this function was called
  if (consumer_id<0) {
    return;
  }

  // if "id" did not change, the consumer did not consume
  // anything neither
  if (consumer_id==previous_consumer_id_){
    return;
  }

  // the consumer did read some data, removing them
  // from the stack, and save their ids in get_consumed_ids
  previous_consumer_id_ = consumer_id;
  items_.remove(nb_to_remove,get_consumed_ids);

  // informing the consumer that we are aware these elements
  // have been consumed
  from_consumer[0]=-1.0;
  from_consumer[1]=0.0;
  shared_memory::set(segment_id_,object_id_consumer_,from_consumer,2);

  // updating item stack in memory
  shared_memory::set(segment_id_,object_id_producer_,
		     items_.get_data(),items_.get_data_size());


}





