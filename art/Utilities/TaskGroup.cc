#include "art/Utilities/TaskGroup.h"

tbb::task_group* art::TaskGroup::instance_{nullptr};
std::mutex art::TaskGroup::m_{};
