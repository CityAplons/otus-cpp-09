#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "log.hpp"
#include "processor.hpp"

class Runner {
  private:
    CommandProcessor proc_;

  public:
    Runner(int blockSize)
        : proc_(CommandProcessor(blockSize, std::make_shared<LegacyPrint>())) {}

    bool DoWork() {
        std::string line;
        while (std::getline(std::cin, line)) {
            proc_.push(line);
        }
        return true;
    }
};
