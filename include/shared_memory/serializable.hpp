

namespace shared_memory {


  class Serializable {

  public:

    virtual void create(double *serialized_data)=0;
    virtual void serialize(double *serialized) const = 0;
    virtual int get_id()=0;

  public:

    virtual static int serialization_size()=0;
    virtual static int get_id(double* serialized_data);
    
  };


}
