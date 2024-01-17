#include "async.hpp"
#include "mq.hpp"

#include <limits.h>
#include <unistd.h>

namespace async {

constexpr static size_t queue_len_ = 10;
struct AsyncHandle {
    otus::StringMQ client;

    AsyncHandle(size_t bulk_len)
        : client(otus::StringMQ("async", otus::StringMQ::EndpointType::Client,
                                queue_len_, bulk_len)) {}
};

handle_t
connect(std::size_t bulk) {
    return new AsyncHandle(bulk);
}

void
receive(handle_t handle, const char *data, std::size_t size) {
    if (handle == nullptr) {
        return;
    }

    AsyncHandle *priv = static_cast<AsyncHandle *>(handle);
    std::string str_data(data, data + size);
    priv->client.send(str_data);
}

void
disconnect(handle_t handle) {
    // TODO: pass handle_t by a reference to be able to invalidate it !!!
    if (handle == nullptr) {
        return;
    }

    AsyncHandle *priv = static_cast<AsyncHandle *>(handle);
    priv->~AsyncHandle();
}

}   // namespace async
