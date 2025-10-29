#include "dnsmessage.hpp"

DNSMessage parse_dns_query(const uint8_t* buffer, size_t len) {
    if (len < 12) {
        throw std::runtime_error("DNS query too short");
    }

    DNSMessage message;

    // DNS header
    message.id = (buffer[0] << 8) | buffer[1];
    message.flags = (buffer[2] << 8) | buffer[3];
    message.question_count = (buffer[4] << 8) | buffer[5];
    message.answer_rrs = (buffer[6] << 8) | buffer[7];
    message.authority_rrs = (buffer[8] << 8) | buffer[9];
    message.additional_rrs = (buffer[10] << 8) | buffer[11];

    if (message.question_count != 1) {
        // On the forum it was said that "ad více dotazů v jedné zprávě DNS: takové zprávy se v praxi nevyskytují, protože není jasná sémantika některých polí v očekávané odpovědi" therefore i forbig queries with more questions
        throw std::runtime_error("DNS query must contain exactly 1 question");
    }

    size_t pos = 12;
    // Parse QNAME
    while (pos < len && buffer[pos] != 0) {
        uint8_t label_len = buffer[pos++];
        if (pos + label_len > len) {
            throw std::runtime_error("Malformed DNS query");
        }
        if (!message.question.query_name.empty()) {
            message.question.query_name.append(".");
        }
        message.question.query_name.append(reinterpret_cast<const char*>(&buffer[pos]), label_len);
        pos += label_len;
    }

    if (pos >= len) {
        throw std::runtime_error("Malformed DNS query");
    }

    pos++; // skip null byte

    if (pos + 4 > len) {
        throw std::runtime_error("Malformed DNS query");
    }

    message.question.query_type = (buffer[pos] << 8) | buffer[pos + 1];
    message.question.query_class = (buffer[pos + 2] << 8) | buffer[pos + 3];
    pos += 4;

    // Debug information
    printf_debug("ID: 0x%04x, Flags: 0x%04x, Questions: %u, Answers: %u, Authority: %u, Additional: %u",
                 message.id, message.flags, message.question_count, message.answer_rrs, 
                 message.authority_rrs, message.additional_rrs);
    printf_debug("Question - QNAME: %s, QTYPE: %u, QCLASS: %u",
                 message.question.query_name.c_str(), message.question.query_type, 
                 message.question.query_class);

    return message;
}

static std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool domain_blocked(std::string qname, const std::vector<std::string>& blocked_domains) {
        
    qname = to_lower(qname);

    // Remove trailing dot if present
    if (!qname.empty() && qname.back() == '.') {
        qname.pop_back();
    }
    
    for (const auto& blocked : blocked_domains) {
        std::string blocked_lower = to_lower(blocked);
        // Exact match
        if (qname == blocked_lower) {
            return true;
        }
        
        // Subdomain match
        if (qname.size() >= blocked_lower.size() + 2) {  // Fixed: >= instead of >
            std::string suffix = "." + blocked_lower;
            if (qname.size() >= suffix.size() &&
                qname.compare(qname.size() - suffix.size(), suffix.size(), suffix) == 0) {
                return true;
            }
        }
    }

    return false; // Domain not blocked
}