
  template <class Serializable>
  Exchange_manager_consumer<Serializable>::Exchange_manager_consumer(std::string segment_id,
								     std::string object_id,
								     int max_exchange_size)
    : items_(max_exchange_size) {

    segment_id_ = segment_id;
    object_id_consumer_ = object_id+"_consumer";
    object_id_producer_ = object_id+"_producer";
    previous_producer_id_=-1;
    ready_to_consume_ = false;
    nb_consumed_ = 0;

    // init of shared memory
    double foo[2];
    foo[0]=static_cast<double>(1);
    foo[1]=static_cast<double>(0);
    shared_memory::set(segment_id_,object_id_consumer_,foo,2);
    shared_memory::set(segment_id_,object_id_producer_,
		       items_.get_data(),items_.get_data_size());

  }


  template <class Serializable>
  Exchange_manager_consumer<Serializable>::~Exchange_manager_consumer(){}


  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::update_memory(){

    static double to_producer[2];

    if (ready_to_consume_ && nb_consumed_>0){
      // items have been consumed, informing the producer
      to_producer[0]=static_cast<double>(items_.get_id());
      to_producer[1]=static_cast<double>(nb_consumed_);
      shared_memory::set(segment_id_,object_id_consumer_,to_producer,2);
      ready_to_consume_ = false;
      nb_consumed_=0;
      return;
    }
    
    // check if the producer read consumed data ...
    shared_memory::get(segment_id_,object_id_consumer_,to_producer,2);
    // which is the case only if to_producer[0] is negative
    if (static_cast<int>(to_producer[0])>0) {
      ready_to_consume_ = false;
      return;
    }

    // the producer removed consumed items from the
    // stack. 
    
    // updating the stack of items from the memory
    shared_memory::get(segment_id_,object_id_producer_,
		       items_.get_data(),
		       items_.get_data_size());

    // first value is an id that increased each time
    // the producer wrote new data in memory

    int id = items_.get_id();
    if (id==previous_producer_id_){
      throw std::runtime_error("exchange_manager_producer removed data, but did not update its id. This is a bug that should not happen.");
    }

    // new values, and the consumer shifted
    // all data to the start of the stack,
    // so reseting the index
    int nb_removed = static_cast<int>(to_producer[1]);
    items_.reset(nb_removed);
    ready_to_consume_=true;
  }

  template <class Serializable>
  bool Exchange_manager_consumer<Serializable>::ready_to_consume(){
    return ready_to_consume_;
  }

  template <class Serializable>
  bool Exchange_manager_consumer<Serializable>::empty(){
    return items_.empty();
  }

  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::consume(Serializable &serializable){
    if(!ready_to_consume_){
      throw std::runtime_error("exchange_manager_consumer tried to consume data when not ready to do so. Please call ready_to_consume(), and only if it returns 'true' call consume().");
    }
    items_.read(serializable);
    nb_consumed_+=1;
  }
