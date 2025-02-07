#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

int main() {
    try {
        io_service io_service;

        std::string server_ip;
        std::cout << "Enter server IP address: ";
        std::cin >> server_ip;

        int port;
        std::cout << "Enter port number: ";
        std::cin >> port;

        tcp::socket socket(io_service);
        socket.connect(tcp::endpoint(ip::address::from_string(server_ip), port));

        std::cout << "Connected to server at " << server_ip << " on port " << port << ". Type messages:\n";

        std::cin.ignore();  

        while (true) {
            std::string message;
            std::getline(std::cin, message);

            if (message == "exit") break;

            write(socket, buffer(message));
        }

    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
