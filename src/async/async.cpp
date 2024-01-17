#include "async.hpp"

namespace async {

handle_t connect(std::size_t bulk) {
    (void)bulk;
    return nullptr;
}

void receive(handle_t handle, const char *data, std::size_t size) {
    (void)handle;
    (void)data;
    (void)size;
}

void disconnect(handle_t handle) {
    (void)handle;
}

}
