#include "network/listener.h"

#include <iostream>

namespace {

const int kWaitTime = 500;
const int kTimeOut = 3000;
const int64_t kConnectionCheckAliveInterval = 1000;

} // namespace

namespace network {

int64_t Listener::VerifySequenceNumber(Listener::Connection& connection, const std::string& host_name, const PackageHeader& header) {
  const int64_t new_sequence_nr = header.sequence_nr();
  const int64_t old_sequence_nr = connection.sequence_nr_;
  const auto gap = new_sequence_nr - old_sequence_nr;

  if (gap < 0 || gap > 1) {
    std::cout << "Gap detected: " << host_name << ", got - " << new_sequence_nr << ", expected - "
              << old_sequence_nr + 1 << std::endl;
  }

  return (gap - 1);
}

void Listener::TerminateTimedOutConnections() {
  for (auto it = connections_.begin(); it != connections_.end();) {
    if (utility::time_in_ms() - it->second.timestamp_ >= kTimeOut) {
      queue_->Push(std::make_pair(it->first, CreatePackage(Request::Leave)));
      std::cout << it->first << " timed out, connection terminated" << std::endl;
      it = connections_.erase(it);
    } else {
      ++it;
    }
  }
}

void Listener::Run() {
  UDPServer server(GetPort());
  Packages packages;
  auto last_timeout_check  = utility::time_in_ms();

  for (;;) {
    if (cancelled_.load(std::memory_order_acquire)) {
      break;
    }
    auto size = server.Receive(&packages, sizeof(packages), kWaitTime);

    if (cancelled_.load(std::memory_order_acquire)) {
      break;
    }
    if ((utility::time_in_ms() - last_timeout_check) >= kConnectionCheckAliveInterval) {
      TerminateTimedOutConnections();
      last_timeout_check = utility::time_in_ms();
    }
    if (size == SOCKET_ERROR) {
      continue;
    }
    if (size < static_cast<ssize_t>(sizeof(packages))) {
      std::cout << "Incomplete package - " << size << std::endl;
      continue;
    }
    if (!packages.header_.VerifyHeader()) {
      std::cout << "Unknown package ignored" << std::endl;
      continue;
    }
    int64_t index = 0;
    std::vector<Package> package_vector;
    const auto& host_name = packages.header_.host_name();

    if (connections_.count(host_name) > 0) {
      auto& connection = connections_.at(host_name);

      index = VerifySequenceNumber(connection, host_name, packages.array_[0].header_);

      if (index > packages.size() || index >= kWindowSize) {
        std::cout << host_name << " has lost too many packages, connection will be terminated" << std::endl;
        connections_.erase(host_name);
        queue_->Push(std::make_pair(host_name, CreatePackage(Request::Leave)));
        continue;
      }
      if (index < 0) {
        std::cout << "Old package(s) ignored" << std::endl;
        continue;
      }
    }
    for (auto i = index; i >= 0; --i) {
      package_vector.push_back(packages.array_[i]);
    }
    for (auto& package : package_vector) {
      bool process_request = true;
      const auto& header = package.header_;

      if (!header.VerifyHeader()) {
        std::cout << "Unknown package ignored" << std::endl;
        continue;
      }
      if (header.request() == Request::Join) {
        if (connections_.count(host_name) == 0) {
          connections_.insert(std::make_pair(host_name, Connection(header.sequence_nr())));
        } else {
          process_request = false;
        }
      } else if (connections_.count(host_name) == 0) {
        continue;
      }
      auto& connection = connections_.at(host_name);

      switch (header.request()) {
        case Request::Leave:
          connections_.erase(host_name);
          break;
        case Request::HeartBeat:
          process_request = false;
          break;
        default:
          break;
      }
      connection.Update(header);
      VerifySequenceNumber(connection, host_name, header);
      if (process_request) {
        queue_->Push(std::make_pair(host_name, package));
        std::cout << host_name << " sent " << ToString(package.header_.request()) << std::endl;
      }
    }
  }
}

} // namespace network
