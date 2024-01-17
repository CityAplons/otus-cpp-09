#pragma once

#include "command.hpp"
#include "print.hpp"

#include <chrono>
#include <format>
#include <fstream>
#include <memory>
#include <queue>
#include <string>

class CommandQueue {
  private:
    std::queue<std::string> fifo_;

  public:
    std::string &GetLastCmd() { return fifo_.front(); }
    void Add(std::string &cmd) {
        if (cmd.size() == 0) {
            return;
        }

        fifo_.push(cmd);
    }

    void ExecuteAll() {
        if (fifo_.size() == 0) {
            return;
        }

        using namespace std::chrono;
        const auto stamp = current_zone()->to_local(system_clock::now());
        const auto unixStamp =
            duration_cast<seconds>(stamp.time_since_epoch()).count();
        std::string fileName =
            std::format("bulk{}.log", std::to_string(unixStamp));
        std::ofstream file(fileName);

        Print out{};
        out.AddStream(std::cout);
        out.AddStream(file);

        out << "bulk:";
        while (!fifo_.empty()) {
            out << ' ' << fifo_.front();
            fifo_.pop();
        }
        out << '\n';
    }
};

class OnBulkAppend : public Command {
  private:
    std::shared_ptr<CommandQueue> queue_;
    std::string data_;

  public:
    OnBulkAppend(std::shared_ptr<CommandQueue> &queue, std::string &cmd)
        : queue_(queue), data_(cmd) {}
    void Execute() override { queue_->Add(data_); }
};

class OnBulkFlush : public Command {
  private:
    std::shared_ptr<CommandQueue> queue_;

  public:
    OnBulkFlush(std::shared_ptr<CommandQueue> &queue) : queue_(queue) {}
    void Execute() override { queue_->ExecuteAll(); }
};