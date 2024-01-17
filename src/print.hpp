#pragma once

#include <iostream>
#include <vector>

struct Print {
    std::vector<std::ostream*> streams;
    void AddStream(std::ostream& stream) { streams.push_back(&stream); }
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