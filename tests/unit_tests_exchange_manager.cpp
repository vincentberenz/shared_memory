#include "shared_memory/exchange_manager_producer.hpp"
#include "shared_memory/exchange_manager_consumer.hpp"
#include "shared_memory/demos/four_int_values.hpp"
#include "shared_memory/tests/tests.h"
#include "gtest/gtest.h"
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <memory>

#define SEGMENT_ID "exchange_manager_ut_segment"
#define OBJECT_ID "exchange_manager_ut_object"
#define BUFFER_SIZE 100

using namespace shared_memory;


static inline void clear_memory(){
  shared_memory::clear_shared_memory(SEGMENT_ID);
}


Exchange_manager_producer<Four_int_values> *producer;
Exchange_manager_consumer<Four_int_values> *consumer;


class Exchange_manager_tests : public ::testing::Test {

protected:

  void SetUp() {
    clear_memory();
    shared_memory::get_segment_mutex(SEGMENT_ID).unlock();
    producer = new Exchange_manager_producer<Four_int_values>(SEGMENT_ID,
							      OBJECT_ID,
							      BUFFER_SIZE);
    consumer = new Exchange_manager_consumer<Four_int_values>(SEGMENT_ID,
							      OBJECT_ID,
							      BUFFER_SIZE);
  }

  void TearDown() {
    delete producer;
    delete consumer;
    clear_memory();
  }
  
};


TEST_F(Exchange_manager_tests,write_and_read){
  Four_int_values p1(1,2,3,4);
  Four_int_values p2(5,6,7,8);
  p1.print();
  p2.print();
  producer->set(p1);
  producer->set(p2);
  producer->write_to_memory();
  consumer->read_memory();
  Four_int_values c1;
  Four_int_values c2;
  consumer->consume(c1);
  consumer->consume(c2);
  c1.print();
  c2.print();
  ASSERT_EQ(true,p1.same(c1));
  ASSERT_EQ(true,p2.same(c2));
}
