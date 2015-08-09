#include "Files.hpp"

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

std::vector<std::string> BTB::GetFilesIn(std::string directory)
{
  DIR *dir;
  dir = opendir(directory.c_str());
  if (dir == NULL)
  {
    throw std::invalid_argument("opendir");
  }

  struct dirent *ent;
  struct stat st;
  std::vector<std::string> result;
  for (errno = 0, ent = readdir(dir); ent != NULL; errno = 0, ent = readdir(dir))
  {
    if (ent->d_name[0] == '.')
    {
      // Ignore hidden files
      continue;
    }

    std::string filename = ent->d_name;
    std::string fullname = directory + "/" + filename;

    if (stat(fullname.c_str(), &st) == -1)
    {
      throw new std::logic_error("stat");
    }
    bool isDirectory = (st.st_mode & S_IFDIR) != 0;
    if (isDirectory)
    {
      continue;
    }

    result.push_back(filename);
  }

  if (errno != 0)
  {
    throw std::logic_error("readdir");
  }

  if (closedir(dir))
  {
    throw std::logic_error("closedir");
  }

  return result;
}
