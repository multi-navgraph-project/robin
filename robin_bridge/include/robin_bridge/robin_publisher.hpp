/**
 * Copyright 2019 INESC TEC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "robin_bridge/robin_publisher.h"
// calls parent constructor and opens robin publisher
template <typename T1, typename T2>
RobinPublisher<T1, T2>::RobinPublisher(ros::NodeHandle &nh, std::string name, bool open, int read_rate)
  : nh_(nh), Robin::Robin(name, sizeof(T1))
{
  if (open)
  {
    this->open(read_rate);
  }
}
// calls parent open(), creates ros publisher and starts read thread
template <typename T1, typename T2>
void RobinPublisher<T1, T2>::open(int read_rate)
{
  Robin::open();
  shm_ptr_ = (T1 *)shared_memory_.ptr_;
  publisher_ = nh_.advertise<T2>(name_, QUEUE_SIZE, LATCH);
  if (read_rate > 0)
  {
    read_thread_ = new std::thread(&RobinPublisher<T1, T2>::publishLoop, this, read_rate);
  }
}
// publishes messages until closing
template <typename T1, typename T2>
void RobinPublisher<T1, T2>::publishLoop(int rate)
{
  ros::Rate ros_rate(rate);
  while (!closing_)
  {
    publish();
    ros_rate.sleep();
  }
}
// publishes shared memory data to topic
template <typename T1, typename T2>
void RobinPublisher<T1, T2>::publish()
{
  if (!isOpen())
  {
    ROS_ERROR("Read failed. Bridge '%s' is not open.", name_.c_str());
    throw 2;
  }
  semaphore_.wait();
  read();
  semaphore_.post();
  
  if (!first)
  {
    old_msg_ = msg_;
    first = true;
  }
  else if (old_msg_ != msg_)
  {
    publisher_.publish(msg_);
    old_msg_ = msg_;
  }
  
}
// reads data from shared memory
template <typename T1, typename T2>
void RobinPublisher<T1, T2>::read()
{
  memcpy(&msg_, shm_ptr_, sizeof(T2));
}
// stops read thread and publisher and calls parent close()
template <typename T1, typename T2>
void RobinPublisher<T1, T2>::close()
{
  if (read_thread_ != NULL)
  {
    closing_ = true;
    read_thread_->join();
    read_thread_ = NULL;
    closing_ = false;
  }
  publisher_.shutdown();
  shm_ptr_ = NULL;
  Robin::close();
}
// closes robin publisher
template <typename T1, typename T2>
RobinPublisher<T1, T2>::~RobinPublisher()
{
  if (isOpen())
  {
    close();
  }
}
