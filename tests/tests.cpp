#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "arguments.hpp"
#include "dnsmessage.hpp"

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
