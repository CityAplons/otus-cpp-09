#pragma once

#include <cassert>
#include <format>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <mqueue.h>
#include <stdlib.h>
#include <string.h>

namespace otus {

class StringMQ {
  public:
    enum EndpointType { Client, Server };

    explicit StringMQ(const std::string &id, EndpointType type,
                      size_t queue_size, size_t msg_size)
        : type_(type) {
        std::string rx_name = "/" + id + "_rx";
        std::string tx_name = "/" + id + "_tx";

        struct mq_attr config;
        struct mq_attr *config_ptr = nullptr;
        if (type_ == EndpointType::Server) {
            mq_unlink(rx_name.c_str());
            mq_unlink(tx_name.c_str());

            config.mq_maxmsg = queue_size;
            config.mq_msgsize = msg_size;
            config_ptr = &config;
        }

        mqd_rx_ = mq_open(rx_name.c_str(), O_RDWR | O_CREAT, 0664, config_ptr);
        assert(mqd_rx_ != -1 &&
               std::format("{} mqueue open failed, errno {}", mqd_rx_, errno).c_str());

        mqd_tx_ = mq_open(tx_name.c_str(), O_RDWR | O_CREAT, 0664, config_ptr);
        assert(mqd_tx_ != -1 &&
               std::format("{} mqueue open failed, errno {}", mqd_tx_, errno).c_str());

        mq_getattr(mqd_rx_, &attr_rx_);
        mq_getattr(mqd_tx_, &attr_tx_);

        if (type_ == EndpointType::Client) {
            return;
        }

        rx_dispather_ = std::thread([this]() {
            while (run_) {
                size_t msgsize = attr_rx_.mq_msgsize;
                char buf[1024];
                // std::string msg;
                // msg.reserve(msgsize);
                memset(buf, '\0', msgsize);
                int result = mq_receive(mqd_rx_, buf, msgsize, &prio_);
                assert(result != -1 &&
                       "Error reading the client message");

                {
                    std::string msg(buf);
                    std::lock_guard<std::mutex> guard(callbacks_lock_);
                    for (auto &&cb : callbacks_) {
                        cb(msg);
                    }
                }
            }
        });
    }

    virtual ~StringMQ() {
        run_ = false;
        if (rx_dispather_.joinable()) {
            rx_dispather_.join();
        }

        mq_close(mqd_rx_);
        mq_close(mqd_tx_);
    }

    void attach(std::function<void(const std::string &)> callback) {
        assert(type_ != EndpointType::Client &&
               "ERROR: Cannot call listen() on client");

        std::lock_guard<std::mutex> guard(callbacks_lock_);
        callbacks_.push_back(callback);
    }

    void send(const std::string &data) {
        int result;
        switch (type_) {
        case EndpointType::Server:
            result = mq_send(mqd_tx_, data.data(), data.size(), prio_);
            break;
        case EndpointType::Client:
            result = mq_send(mqd_rx_, data.data(), data.size(), prio_);
            break;
        default:
            return;
        }

        assert(result != -1 && "Can't send the message");
    }

    std::string read() {
        size_t msgsize = attr_tx_.mq_msgsize;
        std::string msg;

        msg.reserve(msgsize);
        memset(msg.data(), '\0', msg.capacity());
        assert(mq_receive(mqd_tx_, msg.data(), msgsize, &prio_) != -1 &&
               "Error reading the server message");

        return msg;
    }

  private:
    EndpointType type_;
    mqd_t mqd_rx_, mqd_tx_;
    unsigned int prio_ = 1;
    struct mq_attr attr_tx_, attr_rx_;

    bool run_ = true;
    std::thread rx_dispather_;
    std::vector<std::function<void(const std::string &)>> callbacks_;
    std::mutex callbacks_lock_;
};

}   // namespace otus
