#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"

#include <iostream>
#include <vector>

using namespace art::detail;
using art::detail::LinuxProcData;
using art::detail::LinuxProcMgr;

namespace {

  using proc_array = LinuxProcData::proc_array;

  class A {
  public:
    void update() {
      deltas_ = procMgr_.getCurrentData() - data_;
      data_  += deltas_;
    }
    void print() {
      std::cout << " VSIZE: " << data_.at(LinuxProcData::VSIZE) << " " << deltas_.at(LinuxProcData::VSIZE)
                << " RSS: "   << data_.at(LinuxProcData::RSS)   << " " << deltas_.at(LinuxProcData::RSS) << std::endl;
    }
  private:
    LinuxProcMgr procMgr_;
    proc_array data_;
    proc_array deltas_;
  };

}

int main() {

  A procInfo;

  procInfo.update();
  procInfo.print();

  std::vector<int> testVec( 400, 0. );

  procInfo.update();
  procInfo.print();

  testVec.resize( 800, 0. );

  procInfo.update();
  procInfo.print();

  testVec.resize( 1600, 0. );

  procInfo.update();
  procInfo.print();

  std::vector<int> anotherVec( 1000000,0.);

  procInfo.update();
  procInfo.print();

  testVec.resize( 10000, 0. );

  procInfo.update();
  procInfo.print();

}
