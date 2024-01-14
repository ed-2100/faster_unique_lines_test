#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include <hh_c/highwayhash.h>

const uint64_t key[4] = {0xe7f93cac412e38c4, 0xcfaa6c993e2acbd7, 0x55b5fccc63796260, 0x0f9b319eaf7bed1e};

struct HighwayHash
{
  size_t
  operator()(const std::string &s) const
  {
    return HighwayHash64(reinterpret_cast<const uint8_t*>(s.data()), s.size(), key);
  }
};

int
phonyMain(int argc, const char *argv[])
{
  auto t1 = std::chrono::high_resolution_clock::now();

  if (argc != 2)
  {
    throw std::runtime_error("I accept exactly ONE argument!");
  }

  std::ifstream f{argv[1], std::ios::binary};

  // TODO: Implement a more barebones set.
  auto set = std::unordered_set<std::string, HighwayHash>();
#ifndef NDEBUG
  std::chrono::duration<double> getline_time{};
  std::chrono::duration<double> insert_time{};
#endif
  size_t total_count = 0;

  while (!f.eof())
  {
    total_count++;

    std::string line;
#ifndef NDEBUG
    {
      auto t1 = std::chrono::high_resolution_clock::now();
#endif
      std::getline(f, line);
#ifndef NDEBUG
      auto t2 = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double> diff = t2 - t1;

      getline_time = getline_time * (1 - (1. / total_count)) + diff / total_count;
    }

    {
      auto t1 = std::chrono::high_resolution_clock::now();
#endif
      set.insert(line);
#ifndef NDEBUG
      auto t2 = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double> diff = t2 - t1;

      insert_time = insert_time * (1 - (1. / total_count)) + diff / total_count;
    }
#endif
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> ms_double = t2 - t1;

  std::cout << "total lines: " << total_count << "\n"
            << "unique lines: " << set.size() << "\n"
#ifndef NDEBUG
            << "average read time: " << getline_time << "\n"
            << "average insertion time: " << insert_time << "\n"
#endif
            << "total time elapsed: " << ms_double << "\n";

  return 0;
}

int
main(int argc, const char *argv[])
{
  int ret{};

  try
  {
    ret = phonyMain(argc, argv);
  }
  catch (std::runtime_error e)
  {
    std::cerr << "An error has occured: " << e.what() << '\n';
    ret = -1;
  }
  catch (std::exception e)
  {
    std::cerr << "An unknown error has occured.";
  }

  return ret;
}
