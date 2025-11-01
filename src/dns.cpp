#include <iostream>
#include "arguments.hpp"
#include "sock.hpp"
#include "macro.hpp"
#include <sys/select.h>
#include <csignal>

// Signal handler will modify this global flag that controls the main loop
volatile sig_atomic_t keep_running = 1;
bool gverbose = false;

// Handles grecefull stop of the program
void signal_handler(int signum) {
    (void)signum;
    keep_running = 0;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler); // handles ctrl+c
    signal(SIGTERM, signal_handler); // handles termination
    signal(SIGPIPE, SIG_IGN); // ignores sigpipe because of timeout

    try {
        printf_debug("Starting DNS application with %d arguments.", argc);
        // Parses the arguments and prints information for debug
        Arguments args(argc, argv);
        printf_debug("verbose on");
        gverbose = args.verbose; // sets the global verbose flag
        args.resolve_address();
        args.printargs();
        args.load_domains(args.file);
        args.print_blocked_domains();

        // Creates client that opens the sockets and connects to the server
        Client client(args.port, args.resolved_address, args.blocked_domains);

        fd_set read_fds; // file descriptor that will monitor incoming data
        while (keep_running) {
            FD_ZERO(&read_fds); // writes zeros over it
            // sets it to monitor the ipv4 and ipv6 sockets for client queries 
            FD_SET(client.sockipv4, &read_fds);
            FD_SET(client.sockipv6, &read_fds);

            int max_fd = std::max(client.sockipv4, client.sockipv6); // constant for select
            
            // set timeout for 1 second so when keep_running is changed the program can stop for example when ctrl+c is pressed
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            // waits for activity on either socket
            int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);
            
            if (activity < 0) {
                if (errno == EINTR) { // select was interrupted by signal ignore it
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
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return 1;

    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 2;

    } catch (const std::exception& e) {
        std::cerr << "Unhandled std::exception: " << e.what() << std::endl;
        return 3;

    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return 99;
    }

    return 0;
}
