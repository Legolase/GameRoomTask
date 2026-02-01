#include <fstream>
#include <iostream>

#include <RevenuerManager.hpp>
#include <log.hpp>
#include <return_codes.h>

int main(int argc, char** argv)
{
  if (argc != 2) {
    LOG_ERROR() << "Invalid parameter.";
    LOG_ERROR() << "Usage: <program> <input-file>";

    return ERROR_INVALID_PARAMETER;
  }

  std::ifstream in(argv[1]);

  if (!in) {
    LOG_ERROR() << "Input file not found.";
    return ERROR_FILE_NOT_FOUND;
  }

  task::RevenuerManager manager(in, std::cout);

  try {
    manager.process();
  } catch (const std::runtime_error& e) {
    std::cout << e.what() << '\n';
  }
}
