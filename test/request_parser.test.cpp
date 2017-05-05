// Copyright (c) 2017, Christopher Gilbert / DISINT LTD
// All rights reserved.

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <hyperspirit/request_parser.hpp>

typedef std::string::const_iterator                    iterator_type;
typedef client::uri_encoded_data_parser<iterator_type> uri_encoded_data_parser;
typedef client::uri_parser<iterator_type>              uri_parser;
typedef client::request_parser<iterator_type>          request_parser;

TEST_CASE("uri data is correctly decoded", "[uri_parser]") {
    std::string input;
    input  = "of=life%20the%20universe&and=everything";

    iterator_type begin = input.begin();
    iterator_type end = input.end();

    uri_encoded_data_parser grammar;
    std::map<std::string, std::string> data;
    REQUIRE(parse(begin, end, grammar, data));

    REQUIRE(data["of"] ==  "life the universe");
    REQUIRE(data["and"] == "everything");
    REQUIRE(begin ==  end);
}

TEST_CASE("uri's are parsed correctly", "[uri_parser]") {
    uri_parser grammar;
    client::uri uri;

    std::string input;
    input = "/cjgdev/http_parser/api/q?text=ushers#1";

    iterator_type begin = input.begin();
    iterator_type end = input.end();

    REQUIRE(parse(begin, end, grammar, uri));
    REQUIRE(*uri.root ==              "cjgdev");
    REQUIRE(*uri.hierarchy ==         "http_parser/api/q");
    REQUIRE((*uri.queries)["text"] == "ushers");
    REQUIRE(*uri.fragment ==          "1");
    REQUIRE(begin == end);


//    input = "ftp://ftp.is.co.za/rfc/rfc1808.txt";
//    input = "http://www.ietf.org/rfc/rfc2396.txt";
//    input = "ldap://[2001:db8::7]/c=GB?objectClass?one";
//    input = "mailto:John.Doe@example.com";
//    input = "news:comp.infosystems.www.servers.unix";
//    input = "tel:+1-816-555-1212";
//    input = "telnet://192.0.2.16:80/";
//    input = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";
}

TEST_CASE("empty requests are handled correctly", "[request_parser]") {
    std::string input = "";

    iterator_type begin = input.begin();
    iterator_type end = input.end();

    request_parser grammar;
    client::http_request req;
    REQUIRE_FALSE(parse(begin, end, grammar, req));
    REQUIRE(begin == end);
}

TEST_CASE("requests are parsed correctly", "[request_parser]") {
    std::string input;
    input  = "GET /hello.htm HTTP/1.1\x0d\x0a";
    input += "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\x0d\x0a";
    input += "Host: www.google.com\x0d\x0a";
    input += "Accept-Language: en-us\x0d\x0a";
    input += "Accept-Encoding: gzip, deflate\x0d\x0a";
    input += "Connection: Keep-Alive\x0d\x0a";
    input += "\x0d\x0a";

    iterator_type begin = input.begin();
    iterator_type end = input.end();

    request_parser grammar;
    client::http_request req;
    REQUIRE(parse(begin, end, grammar, req));

    REQUIRE(req.method ==                     client::method_t::request_get);
    REQUIRE(*req.uri.root ==                  "hello.htm");
    REQUIRE(req.version ==                    "1.1");
    REQUIRE(req.headers["User-Agent"] ==      "Mozilla/4.0 (compatible; MSIE5.01; Windows NT)");
    REQUIRE(req.headers["Host"] ==            "www.google.com");
    REQUIRE(req.headers["Accept-Language"] == "en-us");
    REQUIRE(req.headers["Accept-Encoding"] == "gzip, deflate");
    REQUIRE(req.headers["Connection"] ==      "Keep-Alive");
    REQUIRE(begin == end);
}

TEST_CASE("strange headers are parsed correctly", "[request_parser]") {
    request_parser grammar;
    client::http_request req;

    std::string input;
    input  = "GET / HTTP/1.1\x0d\x0a";
    input += "aaaaaaa:+++++++\x0d\x0a";
    input += "\x0d\x0a";

    iterator_type begin = input.begin();
    iterator_type end = input.end();
    REQUIRE(parse(begin, end, grammar, req));
    REQUIRE(begin == end);


//    input  = "GET / HTTP/1.1\x0d\x0a";
//    input += "X-SSL-Shenanigans:    -----BEGIN CERTIFICATE-----\x0d\x0a";
//    input += "tMIIFbTCCBFWgAwIBAgICH4cwDQYJKoZIhvcNAQEFBQAwcDELMAkGA1UEBhMCVUsx\x0d\x0a";
//    input += "\x0d\x0a";
//
//    begin = input.begin();
//    end = input.end();
//    REQUIRE(parse(begin, end, grammar, req));
//    REQUIRE(begin == end);
}

TEST_CASE("malformed requests result in an error", "[request_parser]") {
    request_parser grammar;
    client::http_request req;

    std::string input;
    input = "GET / INVALID/0.0\x0d\x0a";
    input += "\x0d\x0a";

    iterator_type begin = input.begin();
    iterator_type end = input.end();
    REQUIRE_FALSE(parse(begin, end, grammar, req));
}