#pragma once

#include "bulk.hpp"
#include "command.hpp"
#include "log.hpp"

class CommandProcessor {
  public:
    CommandProcessor() : queue_(std::make_shared<CommandQueue>()) {
        otus::Log::Get().Info("Bulk created with default block size = {}",
                              blockSize_);
    };

    CommandProcessor(int blockSize)
        : blockSize_(blockSize), queue_(std::make_shared<CommandQueue>()) {
        otus::Log::Get().Info("Bulk created with new block size {}",
                              blockSize_);
    }

    ~CommandProcessor() {
        if (depth_ == 0) {
            OnBulkFlush(queue_).Execute();
        }
    }

    void push(const std::string &line) {
        if (!line.size()) {
            return;
        }

        ++counter_;
        if (line.compare("{") == 0) {
            counter_ = 0;

            if (depth_ == 0) {
                OnBulkFlush(queue_).Execute();
            }
            ++depth_;
        } else if (line.compare("}") == 0) {
            counter_ = 0;

            --depth_;
            if (depth_ == 0) {
                OnBulkFlush(queue_).Execute();
            }
        } else if (line.compare("=") == 0) {
            counter_ = 0;
            otus::Log::Get().Info("Set-size command called");
        } else {
            OnBulkAppend(queue_, line).Execute();
        }

        if (counter_ == blockSize_) {
            OnBulkFlush(queue_).Execute();
            counter_ = 0;
        }
    }

  private:
    int counter_ = 0, depth_ = 0;
    int blockSize_ = 1;

    std::shared_ptr<CommandQueue> queue_;
};