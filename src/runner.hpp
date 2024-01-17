#pragma once

#include <iostream>
#include <string>

#include "async/mq.hpp"
#include "processor.hpp"

class Runner {
  private:
    CommandProcessor proc_;
    otus::StringMQ mqueue_;

  public:
    Runner(int blockSize)
        : proc_(CommandProcessor(blockSize)),
          mqueue_("async", otus::StringMQ::EndpointType::Server, 10, 1024) {}

    bool DoWork() {
        std::string line;
        auto async_callback = [this](const std::string &message) {
            proc_.push(message);
            return;
        };

        mqueue_.attach(async_callback);
        while (std::getline(std::cin, line)) {
            proc_.push(line);
        }
        return true;
    }
};
