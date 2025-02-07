#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <algorithm>

using namespace boost::asio;
using ip::tcp;

std::ofstream logFile("server_log.txt", std::ios::app);  

class Session : public std::enable_shared_from_this<Session> {
private:
    tcp::socket socket_;
    std::vector<std::shared_ptr<Session>>& clients;
    char data_[1024];
    std::string client_address_;

public:
    Session(tcp::socket socket, std::vector<std::shared_ptr<Session>>& clients)
        : socket_(std::move(socket)), clients(clients) {
    }

    void start() {
        try {
            client_address_ = socket_.remote_endpoint().address().to_string();
        }
        catch (std::exception& e) {
            client_address_ = "Unknown";
        }
        log("New client connected: " + client_address_);
        readMessage();
    }

private:
    void readMessage() {
        auto self(shared_from_this());
        socket_.async_read_some(buffer(data_, 1024),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string msg(data_, length);
                    log("Received from " + client_address_ + ": " + msg);
                    broadcast(msg);
                    readMessage();
                }
                else {
                    log("Client disconnected: " + client_address_);
                    clients.erase(std::remove_if(clients.begin(), clients.end(),
                        [this](const std::shared_ptr<Session>& session) {
                            return session.get() == this;
                        }), clients.end());
                }
            });
    }

    void broadcast(const std::string& message) {
        for (auto& client : clients) {
            async_write(client->socket_, buffer(message),
                [](boost::system::error_code, std::size_t) {});
        }
    }

    void log(const std::string& message) {
        std::cout << message << std::endl;
        logFile << message << std::endl;
    }
};

class Server {
private:
    io_service io_service_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> clients;

public:
    Server(short port)
        : acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
        startAccept();
        io_service_.run();
    }

private:
    
    void startAccept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto session = std::make_shared<Session>(std::move(socket), clients);
                clients.push_back(session);
                session->start();
            }
            startAccept();
            });
    }
};

int main() {
    try {
        short port;
        std::cout << "Enter port number to create server on: ";
        std::cin >> port;

        Server server(port);
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
