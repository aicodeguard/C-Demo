#include "WebsocketServer.h"

#include <iostream>
#include <thread>
#include <asio/io_service.hpp>
#include <atomic>

// The port number the WebSocket server listens on
constexpr int PORT_NUMBER = 8080;

void registerCallbacks(WebsocketServer& server, asio::io_service& mainEventLoop) {
    server.connect([&mainEventLoop, &server](ClientConnection conn) {
        mainEventLoop.post([conn, &server]() {
            std::clog << "Connection opened." << std::endl;
            std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;

            // Send a hello message to the client
            server.sendMessage(conn, "hello", Json::Value());
        });
    });

    server.disconnect([&mainEventLoop, &server](ClientConnection conn) {
        mainEventLoop.post([conn, &server]() {
            std::clog << "Connection closed." << std::endl;
            std::clog << "There are now " << server.numConnections() << " open connections." << std::endl;
        });
    });

    server.message("message", [&mainEventLoop, &server](ClientConnection conn, const Json::Value& args) {
        mainEventLoop.post([conn, args, &server]() {
            std::clog << "Message received on main thread:\n";
            for (const auto& key : args.getMemberNames()) {
                std::clog << "\t" << key << ": " << args[key].asString() << std::endl;
            }

            // Echo the message back to the client
            server.sendMessage(conn, "message", args);
        });
    });
}

void startInputThread(WebsocketServer& server, asio::io_service& mainEventLoop, std::atomic<bool>& running) {
    std::thread([&server, &mainEventLoop, &running]() {
        std::string input;
        while (running.load()) {
            if (!std::getline(std::cin, input)) {
                running = false;
                break;
            }

            Json::Value payload;
            payload["input"] = input;
            server.broadcastMessage("userInput", payload);

            mainEventLoop.post([]() {
                std::clog << "User input handled on main thread." << std::endl;
            });
        }
    }).detach();
}

int main() {
    asio::io_service mainEventLoop;
    WebsocketServer server;

    registerCallbacks(server, mainEventLoop);

    std::atomic<bool> running{true};

    std::thread serverThread([&server]() {
        server.run(PORT_NUMBER);
    });

    startInputThread(server, mainEventLoop, running);

    asio::io_service::work work(mainEventLoop);
    mainEventLoop.run();

    running = false;
    serverThread.join();

    return 0;
}
