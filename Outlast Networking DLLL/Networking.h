#pragma once
#include <boost/asio.hpp>
#include "pch.h"

extern "C" __declspec(dllexport) bool ConnectToServer(const char* server_ip, int port);
extern "C" __declspec(dllexport) void SendMessageToServer(const char* message);
extern "C" __declspec(dllexport) void DisconnectFromServer();