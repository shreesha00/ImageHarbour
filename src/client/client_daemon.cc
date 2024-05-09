#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <chrono>
#include <iostream>

#include "../rpc/common.h"
#include "../utils/properties.h"
#include "image_harbour_cli.h"

using namespace imageharbour;

int main(int argc, const char *argv[]) {
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        LOG(ERROR) << "[ImageHarbour]: error creating socket\n";
        return 1;
    }

    std::remove("/imageharbour/daemon");

    sockaddr_un address;
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, "/imageharbour/daemon", sizeof(address.sun_path) - 1);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) == -1) {
        LOG(ERROR) << "[ImageHarbour]: error binding socket\n";
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 10) == -1) {
        LOG(ERROR) << "[ImageHarbour]: error listening on socket\n";
        close(server_socket);
        return 1;
    }

    Properties prop;
    ParseCommandLine(argc, argv, prop);

    ImageHarbourClient cli(prop);

    cli.InitializeConn(prop, prop.GetProperty(PROP_IH_SVR_URI, PROP_IH_SVR_URI_DEFAULT), nullptr);

    LOG(INFO) << "[ImageHarbour]: daemon listening on unix domain socket /imageharbour/daemon\n";

    while (true) {
        sockaddr_un client_address;
        socklen_t client_addr_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_addr_len);
        if (client_socket == -1) {
            LOG(ERROR) << "[ImageHarbour]: error accepting connection\n";
            close(server_socket);
            return 1;
        }

        LOG(INFO) << "[ImageHarbour]: connection accepted from client\n";

        char buffer[512];
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            LOG(ERROR) << "[ImageHarbour]: error receiving data from client\n";
            close(client_socket);
            close(server_socket);
            return 1;
        }
        buffer[bytes_received] = '\0';

        std::string received_data(buffer);
        size_t space_pos = received_data.find(' ');
        std::string image_name = received_data.substr(0, space_pos);
        std::string local_path = received_data.substr(space_pos + 1);

        auto start = std::chrono::high_resolution_clock::now();
        cli.FetchImage(image_name);
        LOG(INFO) << "[ImageHarbour]: time to fetch image " << image_name << " in us: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() -
                                                                           start)
                         .count();

        start = std::chrono::high_resolution_clock::now();
        cli.StoreImage(local_path);
        LOG(INFO) << "[ImageHarbour]: time to store image " << image_name << " in us: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() -
                                                                           start)
                         .count();

        LOG(INFO) << "[ImageHarbour]: image " << image_name << " fetched and stored to " << local_path;

        if (send(client_socket, "OK\n", 3, 0) == -1) {
            LOG(ERROR) << "[ImageHarbour]: error sending response to client\n";
            close(client_socket);
            close(server_socket);
            return 1;
        }
        close(client_socket);

        LOG(INFO) << "[ImageHarbour]: response sent to client\n";
    }

    close(server_socket);

    return 0;
}