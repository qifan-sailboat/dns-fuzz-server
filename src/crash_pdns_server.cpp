#include "auth_server.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>


class PDNSServer : public dns::AuthServer
{
public:
    PDNSServer( const std::string &address, uint16_t port, bool debug )
	: dns::AuthServer( address, port, debug )
    {}

    dns::PacketInfo modifyResponse( const dns::PacketInfo &query, const dns::PacketInfo &original_response, bool via_tcp ) const
    {
	dns::PacketInfo response = original_response;

        Condition cond;
        if ( query.question_section[0].q_type == dns::TYPE_A ) {
            dns::ResourceRecord cname;
            cname.r_domainname = query.question_section[0].q_domainname;
            cname.r_type       = dns::TYPE_CNAME;
            cname.r_class      = dns::CLASS_ANY;
            cname.r_ttl        = 300;
            cname.r_resource_data = dns::RDATAPtr( new dns::RecordCNAME( query.question_section[0].q_domainname ) );
            response.answer_section.push_back( cname );
        }
        
        return response;
    }
};


int main( int argc, char **argv )
{
    namespace po = boost::program_options;

    std::string bind_address;
    uint16_t    bind_port;
    std::string zone_filename;
    std::string apex;
    bool        debug;

    po::options_description desc( "pdns" );
    desc.add_options()( "help,h", "print this message" )

        ( "bind,b", po::value<std::string>( &bind_address )->default_value( "0.0.0.0" ), "bind address" )
        ( "port,p", po::value<uint16_t>( &bind_port )->default_value( 53 ), "bind port" )
	( "file,f", po::value<std::string>( &zone_filename ),           "bind address" )
	( "zone,z", po::value<std::string>( &apex),                     "zone apex" )
        ( "debug,d", po::bool_switch( &debug )->default_value( false ), "debug mode" );
    
    po::variables_map vm;
    po::store( po::parse_command_line( argc, argv, desc ), vm );
    po::notify( vm );

    if ( vm.count( "help" ) ) {
        std::cerr << desc << "\n";
        return 1;
    }

    try {
        PDNSServer server( bind_address, bind_port, debug );
	server.load( apex, zone_filename );
	server.start();
    }
    catch ( std::runtime_error &e ) {
	std::cerr << e.what() << std::endl;
    }
    catch ( std::logic_error &e ) {
	std::cerr << e.what() << std::endl;
    }
    return 0;
}
