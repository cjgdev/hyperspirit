// Copyright (c) 2017, Christopher Gilbert / DISINT LTD
// All rights reserved.

#ifndef HYPERSPIRIT_HTTP_REQUEST_PARSER_HPP
#define HYPERSPIRIT_HTTP_REQUEST_PARSER_HPP

#include <boost/config/warning_disable.hpp>
#include <boost/optional.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <complex>

namespace hyperspirit
{
    namespace http
    {
        namespace qi    = boost::spirit::qi;
        namespace ascii = boost::spirit::ascii;

        enum method_t {
            request_options,
            request_get,
            request_head,
            request_post,
            request_put,
            request_delete,
            request_trace,
            request_connect
        }; // enum method_t

        struct uri_t
        {
            typedef std::map<std::string, std::string> query_type;

            boost::optional<std::string> root;
            boost::optional<std::string> hierarchy;
            boost::optional<query_type>  queries;
            boost::optional<std::string> fragment;
        }; // struct uri_t

        struct request_t
        {
            typedef std::map<std::string, std::string> headers_type;

            method_t     method;
            uri_t        uri;
            std::string  version;
            headers_type headers;
        }; // struct request_t

    } // namespace http
} // namespace hyperspirit

BOOST_FUSION_ADAPT_STRUCT(
        hyperspirit::http::uri_t,
        (boost::optional<std::string>,                          root)
        (boost::optional<std::string>,                          hierarchy)
        (boost::optional<hyperspirit::http::uri_t::query_type>, queries)
        (boost::optional<std::string>,                          fragment)
)

BOOST_FUSION_ADAPT_STRUCT(
        hyperspirit::http::request_t,
        (hyperspirit::http::method_t,                method)
        (hyperspirit::http::uri_t,                   uri)
        (std::string,                                version)
        (hyperspirit::http::request_t::headers_type, headers)
)

namespace hyperspirit
{
    namespace http
    {
        template<typename Iterator>
        struct uri_encoded_data_grammar : qi::grammar<Iterator, std::map<std::string, std::string>()>
        {
            uri_encoded_data_grammar() : uri_encoded_data_grammar::base_type(start)
            {
                using ascii::char_;

                start = query_pair % '&';

                query_pair = url_encoded_string >> '=' >> url_encoded_string;

                url_encoded_string = +(('%' >> encoded_hex) | ~char_("=&# "));
            }

            qi::rule<Iterator, std::map<std::string, std::string>()>  start;
            qi::rule<Iterator, std::pair<std::string, std::string>()> query_pair;
            qi::rule<Iterator, std::string()>                         url_encoded_string;
            qi::uint_parser<char, 16, 2, 2>                           encoded_hex;
        }; // struct uri_encoded_data_grammar

        template<typename Iterator>
        struct uri_grammar : qi::grammar<Iterator, uri_t()>
        {
            uri_grammar() : uri_grammar::base_type(start)
            {
                using qi::lit;
                using ascii::char_;

                // Based on RFC3986
                // host  = https://tools.ietf.org/html/rfc3986#section-3.2.2
                // port  = https://tools.ietf.org/html/rfc3986#section-3.2.3
                // path  = https://tools.ietf.org/html/rfc3986#section-3.3
                // query = https://tools.ietf.org/html/rfc3986#section-3.4

                start = lit('/')
                     >> -(+(~char_("/?# ")))
                     >> -('/' >> +(~char_("?# ")))
                     >> -('?' >> uri_encoded_data)
                     >> -('#' >> +(~char_(' ')))
                      ;
            }

            qi::rule<Iterator, uri_t()>        start;
            uri_encoded_data_grammar<Iterator> uri_encoded_data;
        }; // struct uri_grammar

        struct method_symbols_ : qi::symbols<char, method_t>
        {
            method_symbols_()
            {
                add
                    ("OPTIONS", method_t::request_options)
                    ("GET",     method_t::request_get)
                    ("HEAD",    method_t::request_head)
                    ("POST",    method_t::request_post)
                    ("PUT",     method_t::request_put)
                    ("DELETE",  method_t::request_delete)
                    ("TRACE",   method_t::request_trace)
                    ("CONNECT", method_t::request_connect);
            }
        } method_symbols;

        template<typename Iterator>
        struct request_grammar : qi::grammar<Iterator, request_t()>
        {
            request_grammar() : request_grammar::base_type(start)
            {
                using qi::int_;
                using qi::lit;
                using qi::double_;
                using qi::lexeme;
                using qi::raw;
                using qi::omit;
                using ascii::char_;

                start %= method_symbols >> ' ' >> uri_ref >> ' ' >> http_version >> crlf
                      >> *header
                      >> crlf
                      ;

                crlf = lexeme[lit('\x0d') >> lit('\x0a')];

                lws = omit[-crlf >> *char_(" \x09")];

                http_version = lexeme["HTTP/" >> raw[int_ >> '.' >> int_]];

                header = field_name >> ':' >> lws >> field_value >> crlf;

                field_name = +(~char_("()<>@,;:\\\"/[]?={} \x09"));

                field_value = *(char_ - crlf);
            }

            qi::rule<Iterator, request_t()>                           start;
            uri_grammar<Iterator>                                     uri_ref;
            qi::rule<Iterator>                                        crlf;
            qi::rule<Iterator>                                        lws;
            qi::rule<Iterator, std::string()>                         http_version;
            qi::rule<Iterator, std::pair<std::string, std::string>()> header;
            qi::rule<Iterator, std::string()>                         field_name;
            qi::rule<Iterator, std::string()>                         field_value;
        }; // struct request_grammar

    } // namespace http
} // namespace hyperspirit

#endif // HYPERSPIRIT_HTTP_REQUEST_PARSER_HPP
