#pragma once

#include "bulk.hpp"
#include "command.hpp"

#include <iostream>
#include <string>

class Runner {
  private:
    std::shared_ptr<CommandQueue> queue_;
    int blockSize_;

  public:
    Runner(int blockSize) : blockSize_(blockSize) {
        queue_ = std::make_shared<CommandQueue>();
    }

    bool DoWork() {
        std::string line;
        int counter = 0, depth = 0;
        while (std::getline(std::cin, line)) {
            ++counter;

            if (line.compare("{") == 0) {
                counter = 0;

                if (depth == 0) {
                    OnBulkFlush(queue_).Execute();
                }
                ++depth;
            } else if (line.compare("}") == 0) {
                counter = 0;

                --depth;
                if (depth == 0) {
                    OnBulkFlush(queue_).Execute();
                }
            } else {
                OnBulkAppend(queue_, line).Execute();
            }

            if (counter == blockSize_) {
                OnBulkFlush(queue_).Execute();
                counter = 0;
            }
        }

        if (depth == 0) {
            OnBulkFlush(queue_).Execute();
        }
        return true;
    }
};
