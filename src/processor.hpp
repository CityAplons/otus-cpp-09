#pragma once

#include "bulk.hpp"
#include "command.hpp"
#include "log.hpp"

class CommandProcessor {
  public:
    CommandProcessor()
        : printer_(std::make_shared<LegacyPrint>()),
          queue_(std::make_shared<CommandQueue>(printer_)) {
        otus::Log::Get().Info("Bulk created with default block size = {}",
                              blockSize_);
    };

    CommandProcessor(int blockSize, std::shared_ptr<IPrintable> printer)
        : blockSize_(blockSize), printer_(printer),
          queue_(std::make_shared<CommandQueue>(printer)) {
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
        } else if (line[0] == '=') {
            // Set new block-size by command
            counter_ = 0;
            blockSize_ = std::abs(std::atoi(&line[1]));

            std::string info = std::format("Set-size {}", blockSize_);
            otus::Log::Get().Warn(info);
            printer_->write(info);
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

    std::shared_ptr<IPrintable> printer_;
    std::shared_ptr<CommandQueue> queue_;
};