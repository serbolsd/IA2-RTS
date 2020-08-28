#pragma once
#include <geString.h>
class RTSPlayer
{
 public:
  RTSPlayer() = default;
  ~RTSPlayer() {};

  void
  setID(unsigned int id) {
    m_ID = id;
  };

  unsigned int
  getID() const {
    return m_ID;
  };

  void
  setName(const std::string& name) {
    m_name = name;
  };

  std::string
  getName() const {
    return m_name;
  }

 private:
  unsigned int m_ID = 0;
  std::string m_name;

};
