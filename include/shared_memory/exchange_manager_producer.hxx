
template <class Serializable>
Exchange_manager_producer<Serializable>::Exchange_manager_producer(std::string segment_id,
								   std::string object_id,
								   int max_exchange_size) {

  segment_id_ = segment_id;
  object_id_producer_ = object_id+"_producer";
  object_id_consumer_ = object_id+"_consumer";

  // number of double it takes to serialize an item
  serialization_size_ = Serializable::serialization_size;

  // max exchange size: number of items that fit in the stack,
  // max size : number of doubles the stack should have to fit
  //            this number of items (+2 reserved values)
  max_size_ = max_exchange_size*Serializable::serialization_size;
  max_exchange_size_ = max_exchange_size;
    
  // the stack of data
  data_  = new double[max_size_+2];

  // first reserved value if an id that is increased each time
  // items are written to the memory (to inform the consumer
  // there is something new), second reserved value is the
  // number of items in the stack.
  id_ = 0;
  data_[0] = static_cast<double>(id_);
  data_[1] = static_cast<double>(0); // number of items in data

  // index following where we are in the stack, i.e where
  // next item should be written
  index_ = 1;

  // tmp array for holding serialized values
  serialization_ = new double[serialization_size_];

  // the producer will inform it consumed data by increasing
  // this value
  previous_consumer_id_=-1;

  // once items have been consumed, they are removed from the stack
  // but there id memorized in this queue (to inform user which items
  // have been consumed)
  consumed_.resize(max_exchange_size,-1);


    
}

template <class Serializable>  
Exchange_manager_producer<Serializable>::~Exchange_manager_producer(){
  delete[] data_;
  delete[] serialization_;
}
  

template <class Serializable>
void Exchange_manager_producer<Serializable>::set(const Serializable &serializable){

  // transforming Serializable in
  // list of double
  serializable.serialize(serialization_);

  
  
  // checking this new data will fit in our
  // preallocated memory
  if( (index_+serialization_size_) >= max_size_ ){
    throw std::overflow_error("exchange manager producer: memory overflow when setting a new item");
  }

  // copying the serialized item on top of the stack
  std::memcpy(data_+sizeof(double)*index_,
	      serialization_,
	      serialization_size_*sizeof(double));

  // keeping up with the index
  // from which next data will be copied
  index_ += serialization_size_; 

  // first index is an id, allowing the
  // consumer to check if there is some
  // new data for it to consume
  id_+=1;
  data_[0] = static_cast<double>(id_);
  data_[1] +=1 ; // keeping track number of items in the stack

}


template <class Serializable>
void Exchange_manager_producer<Serializable>::write_to_memory(){
  // writing serialized items to the memory
  shared_memory::set(segment_id_,object_id_producer_,data_,max_size_+2);
}

template<class Serializable>
void Exchange_manager_producer<Serializable>::read_consumer_actions_and_update(){

  // read shared memory to check if the consumer
  // did consume any data
  static int from_consumer[2];

  // format expected: [id, number of items consumed by consumer]
  shared_memory::get(segment_id_,object_id_consumer_,from_consumer,2);
  int consumer_id = from_consumer[0];

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
  // from the stack
      
  previous_consumer_id_ = consumer_id;
  int read_up_to_index = from_consumer[1];

  // the consumer read the data between 2 and read_up_to_index
  // (index 0 and 1 are reserved)

  // saving the ids of consumed items
  for(int i=2;i<read_up_to_index;i+=serialization_size_){
    double *data__ = data_+2*sizeof(double)+sizeof(double)*i;
    int id = data__[Serializable::serialization_id_index];
    consumed_.push_back(id);
  }
      
  // we shift the data to remove consummed content
  std::memcpy(data_+read_up_to_index*sizeof(double),
	      data_+sizeof(double),
	      (index_-read_up_to_index)*sizeof(double));

  // ... and update the index
  index_ -= read_up_to_index;

  // and the number of elements stored in the 
  data_[1] -= index_-read_up_to_index;

  // informing the consumer that we are aware these elements
  // have been consumed
  from_consumer[0]=-1; from_consumer[1]=-1;
  shared_memory::set(segment_id_,object_id_consumer_,from_consumer,2);
    
}

template <class Serializable>
void Exchange_manager_producer<Serializable>::get_and_purge_consumed_ids(std::deque<int> &get){
  while (!consumed_.empty()){
    get.push_back(consumed_.front());
    consumed_.pop_front();
  }
}




