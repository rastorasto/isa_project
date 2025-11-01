#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "macro.hpp"
#include <algorithm>

// DNS Question Structure
struct DNSQuestion {
    std::string query_name;
    uint16_t query_type;
    uint16_t query_class;
};

// DNS Message Structure
// Consists of header and questions
struct DNSMessage {
    uint16_t id;
    uint16_t flags;
    uint16_t question_count;
    uint16_t answer_rrs;
    uint16_t authority_rrs;
    uint16_t additional_rrs;

    DNSQuestion question;
};

// parses dns query and returns the parsed query at DNSMessage structure
DNSMessage parse_dns_query(const uint8_t* buffer, size_t len);
// function that checks if the domain name is blocked
bool domain_blocked(std::string qname, const std::vector<std::string>& blocked_domains);