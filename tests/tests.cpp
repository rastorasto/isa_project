/**
 * Author:  Rastislav Uhliar
 * Login:   xuhliar00 
 *
 * Usage: Unit tests
**/

#define CATCH_CONFIG_MAIN

// #include "catch.hpp"
#include "catch_amalgamated.hpp"
#include "arguments.hpp"
#include "dnsmessage.hpp"

bool gverbose = true;

TEST_CASE("CLI argument parsing", "[cli]") {

    SECTION("All arguments provided") {
        const char *argv[] = {"dns", "-s", "google.com", "-p", "8053", "-f", "filter.txt"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        Arguments args(argc, const_cast<char**>(argv));
        REQUIRE(args.address == "google.com");
        REQUIRE(args.port == 8053);
        REQUIRE(args.file == "filter.txt");
    }

    SECTION("Missing optional port argument") {
        const char *argv[] = {"dns", "-s", "111.111.111.111", "-f", "file.txt"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        Arguments args(argc, const_cast<char**>(argv));
        REQUIRE(args.address == "111.111.111.111");
        REQUIRE(args.port == 53);
        REQUIRE(args.file == "file.txt");
    }

    SECTION("Missing required address argument") {
        const char *argv[] = {"dns", "-p", "8053", "-f", "filter.txt"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        REQUIRE_THROWS_AS(Arguments(argc, const_cast<char**>(argv)), std::invalid_argument);
    }

    SECTION("Invalid port number") {
        const char *argv[] = {"dns", "-s", "example.com", "-p", "invalid_port", "-f", "filter.txt"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        REQUIRE_THROWS_AS(Arguments(argc, const_cast<char**>(argv)), std::invalid_argument);
    }

    SECTION("Inalid port number out of range") {
        const char *argv[] = {"dns", "-s", "example.com", "-p", "65590", "-f", "filter.txt"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        REQUIRE_THROWS_AS(Arguments(argc, const_cast<char**>(argv)), std::invalid_argument);
    }

    SECTION("Missing required file argument") {
        const char *argv[] = {"dns", "-s", "example.com", "-p", "8053"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        REQUIRE_THROWS_AS(Arguments(argc, const_cast<char**>(argv)), std::invalid_argument);
    }

    SECTION("Extra unknown argument") {
        const char *argv[] = {"dns", "-s", "example.com", "-p", "8053", "-f", "filter.txt", "-x"};

        int argc = sizeof(argv) / sizeof(argv[0]);

        REQUIRE_THROWS_AS(Arguments(argc, const_cast<char**>(argv)), std::invalid_argument);
    }

}

TEST_CASE("Blocked domains", "[blocked_domains]"){

    std::vector<std::string> blocked = {"google.com", "youtube.com"};

    SECTION("Domain it not blocked"){
        REQUIRE((domain_blocked("vut.cz", blocked)) == false);
    }

    SECTION("Domain is blocked"){
        REQUIRE((domain_blocked("google.com", blocked)) == true);
    }
    
    SECTION("Subomain is blocked"){
        REQUIRE((domain_blocked("video.youtube.com", blocked)) == true);
    }
}


TEST_CASE("DNS message parsing", "[dns_message]"){
    SECTION("Valid DNS query") {
        // Dns query for test
        uint8_t query[] = {
            0x11, 0x11, // id
            0x01, 0x00, // qr response
            0x00, 0x01, // 1 question
            0x00, 0x00, // answers 0
            0x00, 0x00, // aythority 0
            0x00, 0x00, // additional 0
            0x04, 'm', 'e', 'o', 'w',
            0x03, 'i', 's', 'a', // labels
            0x00,       // null byte ending the name
            0x00, 0x01, // type A
            0x00, 0x01  // class IN
        };
        
        REQUIRE_NOTHROW(parse_dns_query(query, sizeof(query)));
        
        DNSMessage msg = parse_dns_query(query, sizeof(query));
        REQUIRE(msg.id == 0x1111);
        REQUIRE(msg.question_count == 1);
        REQUIRE(msg.question.query_name == "meow.isa");
        REQUIRE(msg.question.query_type == 1);
        REQUIRE(msg.question.query_class == 1);
    }
    
    SECTION("Query too short") {
        uint8_t query[] = {0x12, 0x34, 0x01, 0x00}; 
        REQUIRE_THROWS(parse_dns_query(query, sizeof(query)));
    }
    
    SECTION("Multiple questions rejected") {
        uint8_t query[] = {
            0x22, 0x22, // id
            0x01, 0x00, // qr response
            0x00, 0x05, // 5 questions
            0x00, 0x00, // answers 0
            0x00, 0x00, // aythority 0
            0x00, 0x00, // additional 0
            0x04, 'm', 'e', 'o', 'w',
            0x03, 'i', 's', 'a', // labels
            0x00,       // null byte ending the name
            0x00, 0x01, // type A
            0x00, 0x01  // class IN
        };

        REQUIRE_THROWS(parse_dns_query(query, sizeof(query)));
    }
}
