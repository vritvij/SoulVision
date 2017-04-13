#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <array>
#include <string>
#include <istream>
#include <ostream>
#include <iostream>
#include <Python.h>

#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

using boost::property_tree::ptree; using boost::property_tree::read_json; using boost::property_tree::write_json;

io_service ioservice;
tcp::resolver resolv{ioservice};
tcp::socket tcp_socket{ioservice};
std::array<char, 4096> bytes;

void read_handler(const boost::system::error_code &ec,
                  std::size_t bytes_transferred)
{
    if (!ec)
    {
        std::cout.write(bytes.data(), bytes_transferred);
        std::string strbytes = (std::string)bytes.data();
        //std::cout<<"\nBytes pos: "<<(int)strbytes.find("\r\n\r\n");

        // check if bytes received is body
        if((int)strbytes.find("\r\n\r\n") == -1) {
            ptree root;
            std::stringstream ss;
            ss << bytes.data();
            read_json(ss, root);
            float flee = root.get<float>("fleeProbability");
            std::cout << "\nFlee Probability: " << flee;
        }
        tcp_socket.async_read_some(buffer(bytes), read_handler);
    }
}

void connect_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);

        // JSON Body
        ptree root, info;
        root.put ("deltaLevel", 1);
        root.put ("attackerType", 1);
        root.put ("attackerHealth", 1);
        root.put ("attackerStatus", 1);
        root.put ("defenderType", 1);
        root.put ("defenderHealth", 1);
        root.put ("defenderStatus", 1);
        root.put ("distance", 1);
        //root.put ( "message", "value value: value!");
        int arr[10] = {1,1,1,1,1,1,1,1,1,1};
        for(int i=0; i<10; i++)
        {
            ptree node;
            node.put("", arr[i]); //create node with just value
            info.push_back(std::make_pair("", node)); //add this node to the list
        }
        root.put_child("moveSet", info);
        root.put("fleeProbability", 0);
        root.put("moveProbability", 0);

        std::ostringstream buf;
        write_json (buf, root, false);
        std::string json = buf.str();

        request_stream << "POST /title/ HTTP/1.1 \r\n";
        request_stream << "Host: Localhost" << "\r\n";
        request_stream << "User-Agent: C/1.0";
        request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Content-Length: " << json.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";  //NOTE THE Double line feed
        request_stream << json;

        /*
        std::string r =
                "POST / HTTP/1.1\r\nHost: theboostcpplibraries.com\r\n\r\n";
        */
        write(tcp_socket, request);
        tcp_socket.async_read_some(buffer(bytes), read_handler);
    }
}

void resolve_handler(const boost::system::error_code &ec,
                     tcp::resolver::iterator it)
{
    if (!ec)
        tcp_socket.async_connect(*it, connect_handler);
}

int main()
{
    tcp::resolver::query q{"127.0.0.1", "8081"};
    resolv.async_resolve(q, resolve_handler);
    ioservice.run();
}