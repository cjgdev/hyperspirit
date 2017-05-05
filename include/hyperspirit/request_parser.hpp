// Copyright (c) 2017, Christopher Gilbert / DISINT LTD
// All rights reserved.

#ifndef HTTP_PARSER_REQUEST_PARSER_HPP
#define HTTP_PARSER_REQUEST_PARSER_HPP

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

namespace client
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    enum method_t
    {
        request_options,
        request_get,
        request_head,
        request_post,
        request_put,
        request_delete,
        request_trace,
        request_connect
    };

    struct uri
    {
        typedef std::map<std::string, std::string> query_t;

        boost::optional<std::string> root;
        boost::optional<std::string> hierarchy;
        boost::optional<query_t>     queries;
        boost::optional<std::string> fragment;
    };

    struct http_request
    {
        typedef std::map<std::string, std::string> headers_t;

        method_t    method;
        uri         uri;
        std::string version;
        headers_t   headers;
    };
}

BOOST_FUSION_ADAPT_STRUCT(
        client::uri,
        (boost::optional<std::string>, root)
        (boost::optional<std::string>, hierarchy)
        (boost::optional<client::uri::query_t>, queries)
        (boost::optional<std::string>, fragment)
)

BOOST_FUSION_ADAPT_STRUCT(
        client::http_request,
        (client::method_t, method)
        (client::uri, uri)
        (std::string, version)
        (client::http_request::headers_t, headers)
)

namespace client
{
    template <typename Iterator>
    struct uri_encoded_data_parser : qi::grammar<Iterator, std::map<std::string, std::string>()>
    {
        uri_encoded_data_parser() : uri_encoded_data_parser::base_type(start)
        {
            using ascii::char_;

            start = query_pair % '&';

            query_pair = url_encoded_string >> '=' >> url_encoded_string;

            url_encoded_string = +( ('%' >> encoded_hex) | ~char_( "=&# " ) );
        }

        qi::rule<Iterator, std::map<std::string, std::string>()> start;
        qi::rule<Iterator, std::pair<std::string,std::string>()> query_pair;
        qi::rule<Iterator, std::string()>                        url_encoded_string;
        qi::uint_parser<char, 16, 2, 2>                          encoded_hex;
    };

    template <typename Iterator >
    struct uri_parser : qi::grammar< Iterator, uri() >
    {
        uri_parser() : uri_parser::base_type(start)
        {
            using qi::lit;
            using ascii::char_;

            // Based on RFC3986
            // host  = https://tools.ietf.org/html/rfc3986#section-3.2.2
            // port  = https://tools.ietf.org/html/rfc3986#section-3.2.3
            // path  = https://tools.ietf.org/html/rfc3986#section-3.3
            // query = https://tools.ietf.org/html/rfc3986#section-3.4

            start =
                        lit('/')
                     >> -(+(~char_("/?# ")))
                     >> -('/' >> +(~char_("?# ")))
                     >> -('?' >> uri_encoded_data)
                     >> -('#' >> +(~char_(' ')))
                    ;
        }

        qi::rule<Iterator, uri()>         start;
        uri_encoded_data_parser<Iterator> uri_encoded_data;
    };

    struct method_ : qi::symbols<char, client::method_t>
    {
        method_()
        {
            add
                ("OPTIONS", client::method_t::request_options)
                ("GET",     client::method_t::request_get)
                ("HEAD",    client::method_t::request_head)
                ("POST",    client::method_t::request_post)
                ("PUT",     client::method_t::request_put)
                ("DELETE",  client::method_t::request_delete)
                ("TRACE",   client::method_t::request_trace)
                ("CONNECT", client::method_t::request_connect)
                ;
        }
    } method;

    template <typename Iterator>
    struct request_parser : qi::grammar<Iterator, http_request()>
    {
        request_parser() : request_parser::base_type(start)
        {
            using qi::int_;
            using qi::lit;
            using qi::double_;
            using qi::lexeme;
            using qi::raw;
            using qi::omit;
            using ascii::char_;

            start %=
                       method >> ' ' >> uri_ref >> ' ' >> http_version >> crlf
                    >> *header
                    >> crlf
                    ;

            crlf = lexeme[lit('\x0d') >> lit('\x0a')];

            lws = omit[-crlf >> *char_(" \x09")];

            http_version = lexeme["HTTP/" >> raw[int_ >> '.' >> int_]];

            header = token >> ':' >> lws >> field_value >> crlf;

            token = +(~char_("()<>@,;:\\\"/[]?={} \x09"));

            field_value = *(char_ - crlf);
        }

        qi::rule<Iterator, http_request()>                        start;
        uri_parser<Iterator>                                      uri_ref;
        qi::rule<Iterator>                                        crlf;
        qi::rule<Iterator>                                        lws;
        qi::rule<Iterator, std::string()>                         http_version;
        qi::rule<Iterator, std::pair<std::string, std::string>()> header;
        qi::rule<Iterator, std::string()>                         token;
        qi::rule<Iterator, std::string()>                         field_value;
    };
}

#endif //HTTP_PARSER_REQUEST_PARSER_HPP
