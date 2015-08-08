#include "tcpv4client.hpp"
#include "dns.hpp"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <arpa/inet.h>
#include <boost/numeric/conversion/cast.hpp>

const char *DNS_SERVER_ADDRESS = "192.168.33.10";
// const char *DNS_SERVER_ADDRESS = "172.16.253.81";
// const char *DNS_SERVER_ADDRESS = "49.212.193.254";

int main()
{
    std::vector<dns::QuestionSectionEntry> question_section;
    std::vector<dns::ResponseSectionEntry> answer_section, authority_section, additional_infomation_section;

    dns::QuestionSectionEntry question;
    question.q_domainname = "www.example.com";
    question.q_type       = dns::TYPE_TXT;
    question.q_class      = dns::CLASS_IN;
    question_section.push_back( question );

    std::vector<dns::OptPseudoRROptPtr> options;
    std::string nsid = "";

    options.push_back( dns::OptPseudoRROptPtr( new dns::NSIDOption( nsid ) ) );
    dns::OptPseudoRecord opt;
    opt.payload_size = 1280;
    opt.record_options_data = boost::shared_ptr<dns::ResourceData>( new dns::RecordOptionsData( options ) );

    additional_infomation_section.push_back( dns::generate_opt_pseudo_record( opt ) );

    dns::PacketHeaderField header;
    header.id                   = htons( 1234 );
    header.opcode               = 0;
    header.query_response       = 0;
    header.authoritative_answer = 0;
    header.truncation           = 0;
    header.recursion_desired    = false;
    header.recursion_available  = 0;
    header.zero_field           = 0;
    header.authentic_data       = 0;
    header.checking_disabled    = 0;
    header.response_code        = 0;

    std::vector<uint8_t> packet = dns::generate_dns_packet( header,
							    question_section,
							    answer_section,
							    authority_section,
							    additional_infomation_section );

    tcpv4::ClientParameters tcp_param;
    tcp_param.destination_address = DNS_SERVER_ADDRESS;
    tcp_param.destination_port    = 53;
    tcpv4::Client tcp( tcp_param );
    uint16_t data_size = htons( boost::numeric_cast<uint16_t>( packet.size() ) );
    tcp.send( reinterpret_cast<uint8_t *>(&data_size), 2 );
    tcp.send( packet.data(), packet.size() );

    tcpv4::ConnectionInfo received_packet = tcp.receive_data( 2 );
    int response_size = ntohs( *reinterpret_cast<uint16_t *>( &received_packet.stream[0] ) );
    received_packet = tcp.receive_data( response_size );
    tcp.closeSocket();

    dns::ResponsePacketInfo res = dns::parse_dns_response_packet( received_packet.begin(),
                                                                  received_packet.end() );

    std::cout << res;


    return 0;
}
