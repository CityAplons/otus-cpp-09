#pragma once

#include "print.hpp"

template <size_t pool_size = 1> class Pool {
  private:
    std::array<std::thread, pool_size> threads_;

  public:
    void run(std::function<void(const std::string &)> task,
             const std::string &data) {
        // trivial impl, async seems to be not using the thread pool
        for (auto &&thread : threads_) {
            if (thread.joinable())
                continue;

            thread = std::thread(task, data);
            return;
        }

        for (auto &&thread : threads_) {
            if (!thread.joinable())
                continue;

            thread.join();
            thread = std::thread(task, data);
            return;
        }
    }

    ~Pool() {
        for (auto &&thread : threads_) {
            if (!thread.joinable())
                continue;

            thread.join();
        }
    };
};

class FilePrint : public IPrintable {
  private:
    Pool<2> workers_;

  public:
    void write(const std::string &data) final {
        auto task = [](const std::string &local_data) {
            using namespace std::chrono;
            const auto stamp = current_zone()->to_local(system_clock::now());
            const auto unixStamp =
                duration_cast<seconds>(stamp.time_since_epoch()).count();
            std::string fileName = std::format(
                "bulk{}_{:x}.log", std::to_string(unixStamp), std::rand());

            std::ofstream file(fileName);
            file << local_data;
            file.close();
        };
        workers_.run(task, data);
    };
};

class ConsolePrint : public IPrintable {
  private:
    Pool<1> workers_;

  public:
    void write(const std::string &data) final {
        auto task = [](const std::string &local_data) {
            std::cout << local_data;
            std::flush(std::cout);
        };
        workers_.run(task, data);
    };
};