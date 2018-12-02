#ifndef EXCHANGE_MANAGER_HPP
#define EXCHANGE_MANAGER_HPP



namespace shared_memory {


  class Exchange_manager_consumer {
    
  }
  
  class Exchange_manager_producer {

  public:

    Exchange_manager(std::string segment_id,
		     std::string object_id,
		     int max_exhange_size,
		     int serialization_size) {

      segment_id_ = segment_id;
      object_id_client_ = object_id+"_client";
      object_id_consumer_ = object_id+"_consumer";
      serialization_size_ = serialization_size_;
      max_size_ = max_exchange_size*serialization_size+1
      data_  = new double[max_size_+1];
      id_ = 0;
      data_[0] = static_cast<double>(id_);
      index_ = 1;
      serialization_ = new double[serialization_size_];
      previous_consumer_id_=-1;
      consumed_.resize(max_exchange_size,-1);
      max_exchange_size_ = max_exchange_size;
      
    }

    ~Exchange_manager(){
      delete[] data_;
      delete[] serialization_;
    }
    

    void set(const Serializable &serializable){

      // transforming Serializable in
      // list of double
      serializable.serialize(serialization_);

      // checking this new data will fit in our
      // preallocated memory
      if( (index_+serialization_size) >= max_size_ ){
	throw std::overflow_error();
      }

      // copying the data on top of the stack
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
      
    }

    void set_to_memory(){
      shared_memory.set(segment_id_,object_id_client_,data_,max_size_);
    }

    void read_consumer_actions_and_update(){

      // read shared memory to check if the consumer
      // did consume any data
      static int from_consumer[2];
      shared_memory.get(segment_id_,object_id_consumer_,from_consumer);
      int consumer_id = from_consumer[0];

      // it did not
      if (consumer_id==previous_consumer_id_){
	return;
      }

      // it did, updating data accordingly
      
      previous_consumer_id_ = consumer_id;
      int read_up_to_index = from_consumer[1];

      // the consumer read the data between 1 and read_up_to_index
      // (index 0 is reserved)

      // saving the ids of consumed items
      for(int i=1;i<read_up_to_index_;i+=serialization_size_){
	if(consumed_.size()==(max_exchange_size_-1)){
	  throw std::overflow_error();
	}
	consumed_.push_back(Serialized::get_id())
      }
      
      // we shift the data to remove consummed content

      std::memcpy(data_+read_up_to_index*sizeof(double),
		  data_+sizeof(double),
		  (index_-read_up_to_nindex)*sizeof(double));

      // ... and update the index
      index_ -= read_up_to_index;
      
    }

    void get_and_purge_consummed_ids(std::deque<int> &get){
      while (!consumed_.empty){
	get.push_back(consumed_.front());
	consumed_.pop();
      }
    }
    

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




#endif // EXCHANGE_MANAGER_HPP
