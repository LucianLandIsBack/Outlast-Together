#include "Networking.h"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
using namespace std;


using boost::asio::ip::tcp;

static std::unique_ptr<boost::asio::io_service> g_ioService;
static std::unique_ptr<tcp::socket> g_socket;
static std::unique_ptr<boost::asio::io_service::work> g_work;
static std::thread* g_thread = nullptr;

extern "C" __declspec(dllexport) bool ConnectToServer(const char* server_ip, int port)
{
    try
    {
        if (!g_ioService)
        {
            g_ioService = std::make_unique<boost::asio::io_service>();
            g_work = std::make_unique<boost::asio::io_service::work>(*g_ioService);
            g_thread = new std::thread([]() { g_ioService->run(); });
        }

        tcp::endpoint endpoint(boost::asio::ip::address::from_string(server_ip), port);
        g_socket = std::make_unique<tcp::socket>(*g_ioService);
        g_socket->connect(endpoint);
        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "ConnectToServer error: " << e.what() << std::endl;
        return false;
    }
}

extern "C" __declspec(dllexport) void SendMessageToServer(const char* message)
{
    try
    {
        if (g_socket && g_socket->is_open())
        {
            boost::asio::write(*g_socket, boost::asio::buffer(message, std::strlen(message)));
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "SendMessageToServer error: " << e.what() << std::endl;
    }
}

extern "C" __declspec(dllexport) void DisconnectFromServer()
{
    try
    {
        if (g_socket)
        {
            if (g_socket->is_open())
                g_socket->close();
            g_socket.reset();
        }

        if (g_work)
        {
            g_work.reset();
        }

        if (g_ioService)
        {
            g_ioService->stop();
        }
        if (g_thread)
        {
            if (g_thread->joinable())
                g_thread->join();
            delete g_thread;
            g_thread = nullptr;
        }

        if (g_ioService)
        {
            g_ioService.reset();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "DisconnectFromServer error: " << e.what() << std::endl;
    }
}