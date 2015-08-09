#ifndef __BTB_FILES_H__
#define __BTB_FILES_H__

#include <cerrno>
#include <string>
#include <vector>

namespace BTB {
  std::vector<std::string> GetFilesIn(std::string directory);
}

#endif
