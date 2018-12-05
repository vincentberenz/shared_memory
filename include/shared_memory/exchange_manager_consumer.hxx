
  template <class Serializable>
  Exchange_manager_consumer<Serializable>::Exchange_manager_consumer(std::string segment_id,
								     std::string object_id,
								     int max_exchange_size)
    : items_(max_exchange_size) {

    segment_id_ = segment_id;
    object_id_consumer_ = object_id+"_consumer";
    object_id_producer_ = object_id+"_producer";
    previous_producer_id_=-1;
    ready_to_consume_ = true;
    nb_consumed_ = 0;

    // init of shared memory
    shared_memory::clear_shared_memory(segment_id);
    shared_memory::delete_segment(segment_id);
    double foo[2];
    foo[0]=static_cast<double>(0); 
    foo[1]=static_cast<double>(0);
    shared_memory::set(segment_id_,object_id_consumer_,foo,2);
    shared_memory::set(segment_id_,object_id_producer_,
		       items_.get_data(),items_.get_data_size());
  }


  template <class Serializable>
  Exchange_manager_consumer<Serializable>::~Exchange_manager_consumer(){
    shared_memory::clear_shared_memory(segment_id_);
    shared_memory::delete_segment(segment_id_);
  }


  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::update_memory(){

    // some items have been consumed, we need to inform the producer
    if (nb_consumed_>0){
      static double to_producer[2];
      static int id = 1;
      id++;
      // items have been consumed, informing the producer
      to_producer[0]=static_cast<double>(id);
      to_producer[1]=static_cast<double>(nb_consumed_);
      shared_memory::set(segment_id_,object_id_consumer_,to_producer,2);
      // we need to wait for the producer to update the command stack
      // before we can resume operation
      ready_to_consume_ = false;
      // when operation will be resumed, the stack would have shifted
      // (because of consumed items removal), so updating our pointer
      // in the item stack
      nb_consumed_=0;
      return;
    }

    // we are waiting for the producer to acknowledge
    // it removed consume items from the stack
    if (!ready_to_consume_){
      static double from_producer[2];      
      shared_memory::get(segment_id_,object_id_consumer_,
			 from_producer,2);
      bool consumed_items_removed = (from_producer[0]<0);
      if (consumed_items_removed){
	// the producer removed consumed items,
	// reseting the pointer to the stack and
	// resuming operation
	int nb_removed = static_cast<int>(from_producer[1]);
	items_.reset();
	ready_to_consume_ = true;
      }
    }
    
    // updating stack of items, if operation running
    if(ready_to_consume_){
      shared_memory::get(segment_id_,object_id_producer_,
			 items_.get_data(),
			 items_.get_data_size());
    }
    
  }

  template <class Serializable>
  bool Exchange_manager_consumer<Serializable>::ready_to_consume(){
    return ready_to_consume_;
  }

  template <class Serializable>
  bool Exchange_manager_consumer<Serializable>::empty(){
    bool e = items_.empty();
    return e;
  }

  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::consume(Serializable &serializable){
    if(!ready_to_consume_){
      throw std::runtime_error("exchange_manager_consumer tried to consume data when not ready to do so. Please call ready_to_consume(), and only if it returns 'true' call consume().");
    }
    items_.read(serializable);
    nb_consumed_+=1;
  }
