#include "network.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int create_server_socket(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    // Lets us restart the publisher quickly during testing without
    // hitting "Address already in use" for ~60 seconds.
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port << "\n";
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int accept_client(int server_fd) {
    sockaddr_in client_address{};
    socklen_t client_len = sizeof(client_address);
    int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_len);
    if (client_fd < 0) {
        std::cerr << "Accept failed\n";
        return -1;
    }
    std::cout << "Client connected from " << inet_ntoa(client_address.sin_addr) << "\n";
    return client_fd;
}

int connect_to_server(const std::string& host, int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << "\n";
        close(sock_fd);
        return -1;
    }

    if (connect(sock_fd, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection to " << host << ":" << port << " failed\n";
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}
