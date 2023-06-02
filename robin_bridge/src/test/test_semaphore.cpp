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
#include "robin_bridge/semaphore.h"
#include <gtest/gtest.h>
#include <typeinfo>
class SemaphoreFixture : public ::testing::Test {
protected:
  std::string semaphore_name_ = "test_semaphore";
  std::unique_ptr<Semaphore> semaphore_ = std::unique_ptr<Semaphore>(new Semaphore(semaphore_name_));
  sem_t *semaphore_ptr_ = NULL;
  int semaphore_val_ = -1;
  void SetUp() override
  {
    ASSERT_TRUE(semaphoreIsClosed()) << "Semaphore with same name is already open.";
  }
  bool semaphoreIsClosed()
  {
    return (openSemaphore() == SEM_FAILED && errno == ENOENT);
  }
  sem_t *openSemaphore()
  {
    errno = 0;
    semaphore_ptr_ = sem_open(semaphore_name_.c_str(), 0);
    return semaphore_ptr_;
  }
  bool semaphoreIsOpen()
  {
    return (openSemaphore() != SEM_FAILED);
  }
};
// TEST_F(SemaphoreFixture, construct)  //TODO
// { 
//   // EXPECT_EQ(typeid(std::result_of(Semaphore::Semaphore(semaphore_name_))::type), typeid(Semaphore));
//   // ::testing::StaticAssertTypeEq<Semaphore, std::result_of(Semaphore::Semaphore(semaphore_name_))>();
//   // EXPECT_EQ(typeid(*(new Semaphore::Semaphore(semaphore_name_))), typeid(Semaphore));
//   // semaphore_ = std::make_unique<Semaphore>(semaphore_name_);
//   // EXPECT_EQ(typeid(*semaphore_), typeid(Semaphore));
// }
TEST_F(SemaphoreFixture, open)
{
  EXPECT_TRUE(semaphoreIsClosed()) << "Semaphore opened prematurely.";
  semaphore_->open();
  EXPECT_TRUE(semaphoreIsOpen()) << "Failed to open semaphore. errno " << errno << ": " << strerror(errno);
  EXPECT_EQ(sem_getvalue(semaphore_ptr_, &semaphore_val_), 0) << "Failed to get semaphore value.";
  EXPECT_EQ(semaphore_val_, 1) << "Semaphore opened with value different from 1.";
}
TEST_F(SemaphoreFixture, close)
{
  semaphore_->open();
  EXPECT_TRUE(semaphoreIsOpen()) << "Failed to open semaphore. errno " << errno << ": " << strerror(errno);
  semaphore_->close();
  EXPECT_TRUE(semaphoreIsClosed()) << "Failed to close semaphore.";
}
TEST_F(SemaphoreFixture, destruct)
{
  semaphore_->open();
  EXPECT_TRUE(semaphoreIsOpen()) << "Failed to open semaphore. errno " << errno << ": " << strerror(errno);
  semaphore_.reset();
  EXPECT_TRUE(semaphoreIsClosed()) << "Failed to close semaphore.";
}
TEST_F(SemaphoreFixture, multipleOpenClose)
{
  semaphore_->open();
  EXPECT_TRUE(semaphoreIsOpen()) << "Failed to open semaphore. errno " << errno << ": " << strerror(errno);
  semaphore_->close();
  EXPECT_TRUE(semaphoreIsClosed()) << "Failed to close semaphore.";
  semaphore_->open();
  EXPECT_TRUE(semaphoreIsOpen()) << "Failed to open semaphore. errno " << errno << ": " << strerror(errno);
  semaphore_->close();
  EXPECT_TRUE(semaphoreIsClosed()) << "Failed to close semaphore.";
}
TEST_F(SemaphoreFixture, isOpen)
{
  EXPECT_FALSE(semaphore_->isOpen());
  semaphore_->open();
  EXPECT_TRUE(semaphore_->isOpen());
  semaphore_->close();
  EXPECT_FALSE(semaphore_->isOpen());
}
TEST_F(SemaphoreFixture, wait)
{
  semaphore_->open();
  semaphore_->wait();
  openSemaphore();
  EXPECT_EQ(sem_getvalue(semaphore_ptr_, &semaphore_val_), 0) << "Failed to get semaphore value.";
  EXPECT_EQ(semaphore_val_, 0) << "Semaphore value unchanged after wait.";
}
TEST_F(SemaphoreFixture, post)
{
  semaphore_->open();
  semaphore_->wait();
  semaphore_->post();
  openSemaphore();
  EXPECT_EQ(sem_getvalue(semaphore_ptr_, &semaphore_val_), 0) << "Failed to get semaphore value.";
  EXPECT_EQ(semaphore_val_, 1) << "Semaphore value unchanged after post.";
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
