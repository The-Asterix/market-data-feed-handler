#pragma once
#include <string>

// Creates a TCP server socket bound to the given port and starts listening.
// Returns the listening file descriptor, or -1 on failure.
int create_server_socket(int port);

// Blocks until a client connects. Returns the connected client's
// file descriptor, or -1 on failure.
int accept_client(int server_fd);

// Connects to a TCP server at host:port. Returns the connected
// file descriptor, or -1 on failure.
int connect_to_server(const std::string& host, int port);
