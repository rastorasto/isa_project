# Filtering DNS Resolver

**Author:**     Rastislav Uhliar
**Login:**      xuhliar00
**Date:**       02.11.2025

## Description
The program filters A type queries and blockes the ones from the filter_file. Queries with non-blocked domain are forwarded to the DNS server that is provided in the program arguments. Queries for the blocked domain will receive DNS response with status REFUSED.

## Features
Program supports client connections from both IPv4 and IPv6.
The program has verbose mode for debugging that can be enabled by starting the program with -v flag.
Program also handles graceful shutdown meaning it will close opened sockets and destroy objects when ctrl+c is pressed or the program is terminated.


## Limitations
The upstream DNS server must be reachable with IPv4 (IPv6 clients are supported).
Blocked domain matching uses linear search, could be optimized.
Program accepts only queries with 1 question. (It was said on forum that this is okay)
Program processes queries sequentially. I tested that it can still handle 100 consecutive clients even without implementing query response mapping.
The resolver does not support TCP queries just UDP


## Usage
    dns -s <server> [-p <port>] -f <filter_file> [-v] [-h]

**Parameters:**
- `-s` – IP Address or hostname of upstream DNS resolver  
- `-p` – Port for incoming queries (default: 53)  
- `-f` – Path to filter file containing blocked domains  
- `-h` - Help message 
- `-v` – Verbose mode (prints debug information to stderr)

### Example
    ./dns -s dns.google.com -p 12345 -f blocked.txt

## Submited files
    .
    ├── baddomains.txt          # Example blocklist from assignment
    ├── Makefile
    ├── README.md               # This file
    ├── manual.pdf              # Technical documentation 
    ├── src
    │   ├── arguments.cpp       # Argument parsing implementation
    │   ├── arguments.hpp       # Argument parsing interface
    │   ├── dns.cpp             # Main
    │   ├── dnsmessage.cpp      # DNS protocol implementation
    │   ├── dnsmessage.hpp      # DNS protocol interface
    │   ├── macro.hpp           # Debug macro from IPK Project 2 *
    │   ├── sock.cpp            # Socket implementation
    │   └── sock.hpp            # Socket interface
    └── tests
        ├── catch.hpp           # Catch2 testing framework
        └── tests.cpp           # Unit tests

*  [IPK Project 2: Client for a chat server using the IPK25-CHAT protocol], Macro, Available: https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#example-of-client-logging-in-c