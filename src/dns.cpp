#include <iostream>
#include "arguments.hpp"
#include "sock.hpp"
#include "macro.hpp"
#include <sys/select.h>

volatile sig_atomic_t keep_running = 1;

void signal_handler(int signum) {
    (void)signum;
    keep_running = 0;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    try {
        printf_debug("Starting DNS application with %d arguments.", argc);
        Arguments args(argc, argv);
        args.resolve_address();
        args.printargs();
        args.load_domains(args.file);
        args.print_blocked_domains();

        Client client(args.port, args.resolved_address, args.blocked_domains);

        fd_set read_fds;
        while (keep_running) {
            FD_ZERO(&read_fds);
            FD_SET(client.sockipv4, &read_fds);
            FD_SET(client.sockipv6, &read_fds);

            int max_fd = std::max(client.sockipv4, client.sockipv6);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);
            
            if (activity < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }

            if (activity == 0) {
                continue;
            }

            // Handle client queries on IPv4
            if (FD_ISSET(client.sockipv4, &read_fds)) {
                printf_debug("Reading client IPv4 socket");
                client.handle_client(client.sockipv4);
            }
            
            // Handle client queries on IPv6
            if (FD_ISSET(client.sockipv6, &read_fds)) {
                printf_debug("Reading client IPv6 socket");
                client.handle_client(client.sockipv6);
            }
            }


    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}