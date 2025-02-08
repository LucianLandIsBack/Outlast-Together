#include <boost/asio.hpp>
#include <windows.h>
#include <memory>
#include <thread>
#include <cstring>
#include <iostream>
#include "pch.h"

// Fully qualify types to avoid ambiguity.
using boost::asio::ip::tcp;

// Global variables to manage our networking objects.
static std::unique_ptr<boost::asio::io_service> g_ioService;
static std::unique_ptr<tcp::socket> g_socket;
static std::unique_ptr<boost::asio::io_service::work> g_work;
static std::thread* g_thread = nullptr;

extern "C" __declspec(dllexport) bool ConnectToServer(const char* server_ip, int port)
{
    try
    {
        // Create the io_service and work objects if not already created.
        if (!g_ioService)
        {
            g_ioService = std::make_unique<boost::asio::io_service>();
            // The work object prevents the io_service::run() from returning immediately.
            g_work = std::make_unique<boost::asio::io_service::work>(*g_ioService);
            // Run the io_service in a separate thread.
            g_thread = new std::thread([]() { g_ioService->run(); });
        }

        // Resolve the server address and port.
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(server_ip), port);
        // Create the socket.
        g_socket = std::make_unique<tcp::socket>(*g_ioService);
        // Connect to the server.
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
            // Write the message to the socket.
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

        // Release the work object to allow io_service::run() to exit.
        if (g_work)
        {
            g_work.reset();
        }

        // Stop the io_service and join the background thread.
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

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Optionally, perform per-process initialization here.
        break;
    case DLL_PROCESS_DETACH:
        DisconnectFromServer();
        break;
    }
    return TRUE;
}
