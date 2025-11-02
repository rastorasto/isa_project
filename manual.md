# Filtering DNS Resolver 
## Manual and Technical Documentation
Author: Rastislav Uhliar
Login: xuhliar00
Date 02.11.2025

# Introduction
In this project I implemented a filtering DNS resolver that acts as a middleman between clients DNS queries and an upstream DNS server. The purpose is to block DNS queries for the domains that are provided in a file and to forward others.

## Theory of Operation
### What is Domain Name System (DNS)
The domain name system is a hierarchical and distrubuted name service that provides a naming system for computers. It translates easily remembered domain names like dns.google.com to their IP addresses 8.8.4.4 that are needed to identify computer services and devices.
[https://en.wikipedia.org/wiki/Domain_Name_System]

### DNS Message Structure
The top level format of DNS message is devided into 5 sections. In some cases some of them are empty.

[[todo add image here]]
[[image source https://www.oreilly.com/library/view/hands-on-network-programming/9781789349863/812dd5c5-0d22-4ccd-8faf-f339b416bb2e.xhtml]
    
Header section includes fields that specify which of the remaining sections are presnet and also specifies whether the message is a query or a response, message identifier, opcode that specifies whether message is a standard query or other kind and also response code.
Question section is used to carry the question itself. It contains domain name, type of the query and class.
The answer, authority, and additional sections all share the same format: a variable number of resource records, where the number of records is specified in the corresponding count field in the header.
[https://www.rfc-editor.org/rfc/rfc1035]

### DNS Query Types
DNS supports various record types for different purposes here are some of them.
A type 1    domain name to IPv4 address
NS type 2   authoritative name server
SOA type 6 marks the start of a zone of authority
MX type 15  mail exchange server
PTR type 12 domain name pointer
TXT type 16 text string
AAAA type 28   domain name to IPv6 address
[https://www.rfc-editor.org/rfc/rfc1035]

### DNS Response code
DNS Header includes an RCODE field that specifies the outcome of the query.
0               No error condition
1               Format error - The name server was
                unable to interpret the query.
2               Server failure - The name server was
                unable to process this query due to a
                problem with the name server.
3               Name Error - Meaningful only for
                responses from an authoritative name
                server, this code signifies that the
                domain name referenced in the query does
                not exist.
4               Not Implemented - The name server does
                not support the requested kind of query.
5               Refused - The name server refuses to
                perform the specified operation for
                policy reasons.  For example, a name
                server may not wish to provide the
                information to the particular requester,
                or a name server may not wish to perform
                a particular operation (e.g., zone
                transfer) for particular data.
6-15            Reserved for future use.
[https://www.rfc-editor.org/rfc/rfc1035]

---
## Usage
After compiling the program with `make` it can be run using.
    ./dns -s <server> [-p <port>] -f <filter_file> [-v] [-h]

**Parameters:**
- `-s` – IP Address or hostname of upstream DNS resolver  
- `-p` – Port for incoming queries (default: 53)  
- `-f` – Path to filter file containing blocked domains  
- `-h` - Help message 
- `-v` – Verbose mode (prints debug information to stderr)

### Example
    ./dns -s dns.google.com -p 12345 -f blocked.txt

---

## Implementation Details
### Code Structure
Below is output of `tree` command in project directory with comments regarding files.
```bash
tree
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

```

>macro.hpp [5](#ref5)

### How it works
#### Setup
The program starts by creating instance of Arguments structure that parses arguments from command line and saves them. It also creates a vector of strings that contains the blocked domains. If domain name was provided in the arguments instead of IP address it resolves it. (arguments.hpp)

After this the program creates a Client instance with information from the arguments (port, resolved_address and blocked_domains). (socks.hpp)
In the constructor the Client creates 2 sockets for comunicating with the user. One socket for IPv4 and the other one for IPv6. After they are created they are binded to the port that was specified in the arguments. Next the last socket is created, this one is used for communication with the upstream server. For this socket timeout is set so if the server doesn't respond the program doesn't freeze. The program runs in a while loop and i use select to monitor activity on both client sockets. Select also has timeout so it can check the while expression that will change when the ctrl+c is pressed or program is terminated in other way. This is so the program has gracefull stop and frees the objects and closes the sockets as it should. (dns.hpp main)

#### Query handling
When client sends a query the select changes the file descriptor and client instance is called with function handle_client (sock.hpp). The function receives the query from the client using recvfrom and checks if the query was sent from IPv4 of IPv6 interface and sets the structure sockaddr_storage. After this the parse_dns_query is called.
This function parses the dns query and saves the result into DNSMessage structure (dnsmessage.hpp). The function checks if the DNS query if malformed or that it must contain exactly 1 question and if it doesn't exception is throwed. On the forum it was said that more questions in 1 DNS query are generaly not used. Then the parsed DNS query named is compared with the blocked_domains using domain_blocked function. After this if the domain is blocked function send_refused is called that creates a DNS Response header with status REFUSED and sends it to the client. If the domain was not blocked the forward_to_upstream function is called that forwards the DNS Query from the client to the upstream dns server and waits for the response from the upstream server. If the server doesn't respond the timout is triggered. Otherwise the response from the server is forwarded back to the client. Since the buffer is big enough the program proccesses the queries sequentially like described here. I tested the program with 100 consecutive client queries and it worked fine.

---

## Testing
### Test environment

I developed the program on macOS but tested on eva and merlin as well. All tests passed on all platforms


### Unit Tests
The program was tested with Catch Unit Testing Framework. I decided to use catch because it needs just a single header file to work. Also i read about it in *C++ Crash Course* [4](#ref4).

### List of all unit tests
```c++
TEST_CASE("CLI argument parsing", "[clil") {
    SECTION("All arguments provided")
    SECTION ("Missing optional port argument")
    SECTION( "Missing required address argument")
    SECTION("Invalid port number")
    SECTION ("Inalid port number out of range")
    SECTION ("Missing required file argument")
    SECTION ("Extra unknown argument")
}

TEST_CASE("Blocked domains", "[blocked_domains]"){
SECTION ("Domain it not blocked" )
SECTION("Domain is blocked")
SECTION ("Subomain is blocked") 
}

TEST_CASE ("DNS message parsing", "[dns_message]"){
    SECTION("Valid DNS query")
    SECTION("Query too short")
    SECTION ("Multiple questions rejected")
}
```

### What was tested
I tested the program with unit tests as well as manual testing. When using the -v verbose flag the program prints all important information that was usefull when testing the program manually. The program was tested on eva and merlin with unit tests as well as manually by starting the program and using `dig` to send the queries. The program passed all of the tests.

```bash
# Trying working domain 
xuhliar00@merlin: ~$ dig +short @localhost -p 15000 google.com
142.251.36.110

# Trying blocked domain
xuhliar00@merlin: ~$ dig @localhost -p 15000 baddomain.org | grep status
;; ->>HEADER<<- opcode: QUERY, status: REFUSED, id: 22058
xuhliar00@merlin: ~$ 
```


```bash
# Debug information in the program when using -v verbose flag
# ./dns -s dns.google.com -p 15000 -f small_bad_domains.txt -v

src/dns.cpp:73   |            main | Reading client IPv6 socket
src/sock.cpp:223  |   handle_client | [IPv6] From [::1]:53023 — 51 bytes
src/dnsmessage.cpp:53   | parse_dns_query | ID: 0xfc37, Flags: 0x0120, Questions: 1, Answers: 0, Authority: 0, Additional: 1
src/dnsmessage.cpp:56   | parse_dns_query | Question - QNAME: google.com, QTYPE: 1, QCLASS: 1
src/dnsmessage.cpp:70   |  domain_blocked | checking if domain is blocked
src/sock.cpp:241  |   handle_client | Domain google.com is ALLOWED, forwarding to upstream
src/sock.cpp:174  | forward_to_upstream | Forwarded query to upstream server (51 bytes)
src/sock.cpp:188  | forward_to_upstream | Received response from upstream server (55 bytes)
src/sock.cpp:196  | forward_to_upstream | Forwarded response to client (55 bytes)

src/dns.cpp:73   |            main | Reading client IPv6 socket
src/sock.cpp:223  |   handle_client | [IPv6] From [::1]:36514 — 54 bytes
src/dnsmessage.cpp:53   | parse_dns_query | ID: 0x3f0e, Flags: 0x0120, Questions: 1, Answers: 0, Authority: 0, Additional: 1
src/dnsmessage.cpp:56   | parse_dns_query | Question - QNAME: baddomain.org, QTYPE: 1, QCLASS: 1
src/dnsmessage.cpp:70   |  domain_blocked | checking if domain is blocked
src/sock.cpp:237  |   handle_client | Domain baddomain.org is BLOCKED
src/sock.cpp:150  |    send_refused | Sent REFUSED response (31 bytes)
```

---

### Valgrind
When running valgrind on server merlin and querying for domain after exiting the program valgrind shows no leaks.
```
==6604== 
==6604== HEAP SUMMARY:
==6604==     in use at exit: 0 bytes in 0 blocks
==6604==   total heap usage: 6,103 allocs, 6,103 frees, 581,928 bytes allocated
==6604== 
==6604== All heap blocks were freed -- no leaks are possible
==6604== 
==6604== For lists of detected and suppressed errors, rerun with: -s
==6604== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
xuhliar00@merlin: ~/isa_project$ 
```
---

## Limitations
- The upstream DNS server must be reachable via IPv4 (IPv6 clients are supported).

- Blocked domain matching uses linear search — not optimal for very large lists.

- Only single-question DNS queries are accepted. (It was said on forum that this is okay)

- Program processes queries sequentially. I tested that it can still handle 100 consecutive clients even without implementing query response mapping.

- The resolver does not support TCP queries just UDP

## Bibliography

<a id="ref5"></a> [5]: [IPK Project 2: Client for a chat server using the IPK25-CHAT protocol], Macro, Available: https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#example-of-client-logging-in-c
