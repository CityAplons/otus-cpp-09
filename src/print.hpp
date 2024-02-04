#pragma once

#include <chrono>
#include <cstdlib>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class IPrintable {
  public:
    virtual void write(const std::string &) = 0;
    virtual ~IPrintable() {}
};

struct Print {
    std::vector<std::ostream *> streams;
    void AddStream(std::ostream &stream) { streams.push_back(&stream); }
    void RemoveStream() { streams.pop_back(); }
};

template <class T>
Print &
operator<<(Print &fork, T data) {
    for (auto &&stream : fork.streams) {
        *stream << data;
    }
    return fork;
}

Print &
operator<<(Print &fork, std::ostream &(*f)(std::ostream &) ) {
    for (auto &&stream : fork.streams) {
        *stream << f;
    }
    return fork;
}

template <class T>
Print &
operator<<(Print &fork, std::ostream &(*f)(std::ostream &, T)) {
    for (auto &&stream : fork.streams) {
        *stream << f;
    }
    return fork;
}

class LegacyPrint : public IPrintable {
  private:
    Print out_;

  public:
    LegacyPrint() noexcept { out_.AddStream(std::cout); }

    void write(const std::string &data) final {
        using namespace std::chrono;
        const auto stamp = current_zone()->to_local(system_clock::now());
        const auto unixStamp =
            duration_cast<seconds>(stamp.time_since_epoch()).count();
        std::string fileName =
            std::format("bulk{}.log", std::to_string(unixStamp));

        std::ofstream file(fileName);

        out_.AddStream(file);
        out_ << data;
        out_.RemoveStream();

        file.close();
    };
};