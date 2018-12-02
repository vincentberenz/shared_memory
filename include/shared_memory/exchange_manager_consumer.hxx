
  template <class Serializable>
  Exchange_manager_consumer<Serializable>::Exchange_manager_consumer(std::string segment_id,
						       std::string object_id,
						       int max_exchange_size){

    segment_id_ = segment_id;
    object_id_consumer_ = object_id+"_consumer";
    object_id_producer_ = object_id+"_producer";
    serialization_size_ = Serializable::serialization_size;
    max_size_ = max_exchange_size*serialization_size_;
    data_  = new double[max_size_+2];
    max_exchange_size_ = max_exchange_size;
    previous_producer_id_=-1;
    consumer_id_=-1;
    index_ = 2;
    nb_elements_=0;
    nb_consumed_=0;
    
  }


  template <class Serializable>
  Exchange_manager_consumer<Serializable>::~Exchange_manager_consumer(){
    delete[] this->data_;
  }


  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::read_memory(){
    // reading the stack of items from the memory
    shared_memory::get(segment_id_,object_id_producer_,data_,max_size_+2);
    // first value is an id that increased each time
    // the producer wrote new data in memory
    int id = data_[0];
    // second value is the number of items stored in the
    // stack
    // no new values, so nothing to update
    if (id==previous_producer_id_){
      return;
    }
    // new values, and the consumer shifted
    // all data to the start of the stack,
    // so reseting the index
    previous_producer_id_ = id;
    index_=2;
    nb_elements_  = data_[1];
  }

  template <class Serializable>
  bool Exchange_manager_consumer<Serializable>::empty(){
    return (nb_consumed_==nb_elements_);
  }

  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::consume(Serializable &serializable){
    // assumes not empty !
    serializable.create(data_+sizeof(double)*nb_consumed_*Serializable::serialization_size);
    nb_consumed_++;
  }

  template <class Serializable>
  void Exchange_manager_consumer<Serializable>::write_to_memory(){
    // values read will be either [-1,-1], indicating
    // producer already read info about consummed data or
    // values written by this function, indicating the
    // producer did not read the previously written data
    // yet.
    static int to_producer[2];
    // check if the producer already read consumed data ...
    shared_memory::get(segment_id_,object_id_consumer_,to_producer,2);
    // which is the case of to_producer[0] is negative
    if (to_producer[0]>0) {
      // producer did not check yet consumed data
      return;
    }
    // producer ready to read which data has been consumed
    // format [id, number of items consumed]
    consumer_id_++;
    to_producer[0]=consumer_id_;
    to_producer[1]=nb_consumed_;
    shared_memory::set(segment_id_,object_id_consumer_,to_producer,2);
    // reseting the number of items that have been consummed
    nb_consumed_ = 0;
  }


