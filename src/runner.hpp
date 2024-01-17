#pragma once

#include <iostream>
#include <string>

#include "processor.hpp"

class Runner {
  private:
    CommandProcessor proc_;

  public:
    Runner(int blockSize) : proc_(CommandProcessor(blockSize)) {}

    bool DoWork() {
        std::string line;
        while (std::getline(std::cin, line)) {
            proc_.push(line);
        }
        return true;
    }
};
