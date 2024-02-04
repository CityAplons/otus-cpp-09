#pragma once

#include <map>
#include <sstream>
#include <string>

#include "../log.hpp"
#include "../processor.hpp"

namespace otus {

struct Client {
    CommandProcessor proc;
    std::string buffer;
};

class ConcurrentRunner {
  private:
    std::map<int, Client> clients_;

  public:
    using runner_t = int *;

    ConcurrentRunner() {}

    runner_t connect(size_t bulk) {
        runner_t handle = new int(0);
        auto latest_client = clients_.rbegin();
        if (latest_client != clients_.rend()) {
            *handle = latest_client->first + 1;
        }

        int id = *handle;
        Client instance = {
            CommandProcessor(bulk, std::make_shared<LegacyPrint>()),
            ""
        };
        clients_.emplace(std::make_pair(id, std::move(instance)));
        otus::Log::Get().Info("(Connect) New [{}] client connected", id);
        return handle;
    }

    void disconnect(runner_t handle) {
        if (handle && !clients_.count(*handle)) {
            otus::Log::Get().Error("(Disconnect) Bad handle");
            return;
        }

        clients_.erase(*handle);
        delete handle;
    }

    bool receive(runner_t handle, const char *data, size_t len) {
        if (handle && !clients_.count(*handle)) {
            otus::Log::Get().Error("(Receive) Bad handle");
            return false;
        }

        int id = *handle;
        std::string received(data, data + len);
        clients_[id].buffer += received;
        if (received.find('\n') == std::string::npos) {
            otus::Log::Get().Debug("(Receive) chunk received");
            return true;
        }

        std::string filtered;
        std::stringstream sstream(clients_[id].buffer);
        while (std::getline(sstream, filtered, '\n')) {
            clients_[id].proc.push(filtered);
        }

        auto reminder_begin = clients_[id].buffer.find_last_of('\n') + 1;
        clients_[id].buffer = clients_[id].buffer.substr(reminder_begin);
        return true;
    }
};

}   // namespace otus
