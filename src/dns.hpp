#ifndef DNS_HPP
#define DNS_HPP

#include "utils.hpp"
#include "domainname.hpp"
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

namespace dns
{
    typedef std::vector<uint8_t>::iterator       PacketIterator;
    typedef std::vector<uint8_t>::const_iterator ConstPacketIterator;

    typedef uint8_t Opcode;
    const Opcode    OPCODE_QUERY  = 0;
    const Opcode    OPCODE_NOTIFY = 4;
    const Opcode    OPCODE_UPDATE = 5;

    typedef uint16_t Class;
    const Class      CLASS_IN      = 1;
    const Class      CLASS_ANY     = 255;
    const Class      UPDATE_NONE   = 254;
    const Class      UPDATE_EXIST  = 255;
    const Class      UPDATE_ADD    = 1;
    const Class      UPDATE_DELETE = 255;

    typedef uint16_t Type;
    const Type       TYPE_A      = 1;
    const Type       TYPE_NS     = 2;
    const Type       TYPE_CNAME  = 5;
    const Type       TYPE_SOA    = 6;
    const Type       TYPE_MX     = 15;
    const Type       TYPE_TXT    = 16;
    const Type       TYPE_KEY    = 25;
    const Type       TYPE_AAAA   = 28;
    const Type       TYPE_NAPTR  = 35;
    const Type       TYPE_DNAME  = 39;
    const Type       TYPE_OPT    = 41;
    const Type       TYPE_APL    = 42;
    const Type       TYPE_DS     = 43;
    const Type       TYPE_RRSIG  = 46;
    const Type       TYPE_DNSKEY = 48;
    const Type       TYPE_TLSA   = 52;
    const Type       TYPE_TKEY   = 249;
    const Type       TYPE_TSIG   = 250;
    const Type       TYPE_IXFR   = 251;
    const Type       TYPE_AXFR   = 252;
    const Type       TYPE_ANY    = 255;

    typedef uint32_t TTL;

    typedef uint16_t OptType;
    const OptType    OPT_NSID          = 3;
    const OptType    OPT_CLIENT_SUBNET = 8;

    typedef uint8_t    ResponseCode;
    const ResponseCode NO_ERROR       = 0;
    const ResponseCode NXRRSET        = 0;
    const ResponseCode FORMAT_ERROR   = 1;
    const ResponseCode SERVER_ERROR   = 2;
    const ResponseCode NAME_ERROR     = 3;
    const ResponseCode NXDOMAIN       = 3;
    const ResponseCode NOT_IMPLEENTED = 4;
    const ResponseCode REFUSED        = 5;
    const ResponseCode BADSIG         = 16;
    const ResponseCode BADKEY         = 17;
    const ResponseCode BADTIME        = 18;

    class ResourceData;
    typedef boost::shared_ptr<ResourceData> ResourceDataPtr;

    class ResourceData
    {
    public:
        virtual ~ResourceData()
        {
        }

        virtual std::string toString() const                       = 0;
        virtual void outputWireFormat( WireFormat &message ) const = 0;
        virtual Type     type() const                              = 0;
        virtual uint16_t size() const                              = 0;
    };

    class RecordRaw : public ResourceData
    {
    private:
        uint16_t             rrtype;
        std::vector<uint8_t> data;

    public:
        RecordRaw( uint8_t t, const std::vector<uint8_t> &d ) : rrtype( t ), data( d )
        {
        }

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual Type type() const
        {
            return rrtype;
        }
        virtual uint16_t size() const
        {
            return data.size();
        }
    };

    class RecordA : public ResourceData
    {
    private:
        uint32_t sin_addr;

    public:
        RecordA( uint32_t in_sin_addr );
        RecordA( const std::string &in_address );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual Type type() const
        {
            return TYPE_A;
        }
        virtual uint16_t size() const
        {
            return sizeof( sin_addr );
        }
        static ResourceDataPtr parse( const uint8_t *begin, const uint8_t *end );
    };

    class RecordAAAA : public ResourceData
    {
    private:
        uint8_t sin_addr[ 16 ];

    public:
        RecordAAAA( const uint8_t *sin_addr );
        RecordAAAA( const std::string &address );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual Type type() const
        {
            return TYPE_AAAA;
        }
        virtual uint16_t size() const
        {
            return sizeof( sin_addr );
        }

        static ResourceDataPtr parse( const uint8_t *begin, const uint8_t *end );
    };

    class RecordNS : public ResourceData
    {
    private:
        Domainname domainname;
        Offset     offset;

    public:
        RecordNS( const Domainname &name, Offset off = NO_COMPRESSION );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_NS;
        }
        virtual uint16_t size() const
        {
            return domainname.size( offset );
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordMX : public ResourceData
    {
    private:
        uint16_t   priority;
        Domainname domainname;
        Offset     offset;

    public:
        RecordMX( uint16_t pri, const Domainname &name, Offset off = NO_COMPRESSION );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_MX;
        }
        virtual uint16_t size() const
        {
            return sizeof( priority ) + domainname.size( offset );
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordTXT : public ResourceData
    {
    private:
        std::vector<std::string> data;

    public:
        RecordTXT( const std::string &data );
        RecordTXT( const std::vector<std::string> &data );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_TXT;
        }
        virtual uint16_t size() const;

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordCNAME : public ResourceData
    {
    private:
        Domainname domainname;
        uint16_t   offset;

    public:
        RecordCNAME( const Domainname &name, uint16_t off = NO_COMPRESSION );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_CNAME;
        }
        virtual uint16_t size() const
        {
            return domainname.size( offset );
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordNAPTR : public ResourceData
    {
    private:
        uint16_t    order;
        uint16_t    preference;
        std::string flags;
        std::string services;
        std::string regexp;
        Domainname  replacement;
        uint16_t    offset;

    public:
        RecordNAPTR( uint16_t           in_order,
                     uint16_t           in_preference,
                     const std::string &in_flags,
                     const std::string &in_services,
                     const std::string &in_regexp,
                     const Domainname & in_replacement,
                     uint16_t           in_offset = NO_COMPRESSION );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_NAPTR;
        }
        virtual uint16_t size() const;

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordDNAME : public ResourceData
    {
    private:
        Domainname domainname;
        uint16_t   offset;

    public:
        RecordDNAME( const Domainname &name, uint16_t off = NO_COMPRESSION );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_DNAME;
        }
        virtual uint16_t size() const
        {
            return domainname.size( offset );
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordSOA : public ResourceData
    {
    private:
        Domainname mname;
        Domainname rname;
        uint32_t   serial;
        uint32_t   refresh;
        uint32_t   retry;
        uint32_t   expire;
        uint32_t   minimum;
        Offset     mname_offset;
        Offset     rname_offset;

    public:
        RecordSOA( const Domainname &mname,
                   const Domainname &rname,
                   uint32_t          serial,
                   uint32_t          refresh,
                   uint32_t          retry,
                   uint32_t          expire,
                   uint32_t          minimum,
                   Offset            moff = NO_COMPRESSION,
                   Offset            roff = NO_COMPRESSION );

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_SOA;
        }
        virtual uint16_t size() const;

        const std::string getMName() const
        {
            return mname.toString();
        }
        const std::string getRName() const
        {
            return rname.toString();
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };


    struct APLEntry {
        uint16_t   address_family;
        uint8_t    prefix;
        bool       negation;
        PacketData afd;
    };

    class RecordAPL : public ResourceData
    {
    private:
        std::vector<APLEntry> apl_entries;

    public:
        static const uint16_t IPv4    = 1;
        static const uint16_t IPv6    = 2;
        static const uint16_t Invalid = 0xffff;

        RecordAPL( const std::vector<APLEntry> &in_apls ) : apl_entries( in_apls )
        {
        }

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_APL;
        }
        virtual uint16_t size() const;

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordRRSIG : public ResourceData
    {
    private:
        Type     type_covered;
        uint8_t  algorithm;
        uint8_t  label_count;
        uint32_t original_ttl;
        uint32_t expiration;
        uint32_t inception;
        uint16_t key_tag;
        Domainname signer;
        std::vector<uint8_t> signature;

    public:
        static const uint16_t SIGNED_KEY = 1 << 7;
        static const uint8_t  RSAMD5     = 1;
        static const uint8_t  RSASHA1    = 5;
        static const uint8_t  RSASHA256  = 8;
        static const uint8_t  RSASHA512  = 10;

        RecordRRSIG( Type                       t,
                     uint8_t                    algo,
                     uint8_t                    label,
                     uint32_t                   ttl,
                     uint32_t                   expire,
                     uint32_t                   incept,
                     uint16_t                   tag,
                     const Domainname           &sign,
                     const std::vector<uint8_t> &sig )
            : type_covered( t ),
              algorithm( algo ),
              label_count( label ),
              original_ttl( ttl ),
              expiration( expire ),
              inception( incept ),
              key_tag( tag ),
              signer( sign ),
              signature( sig ) 
        {
        }

        virtual std::string toString() const
        {
            return "";
        }

        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t size() const
        {
            return 2 +  // type_covered(uint16_t)
                   1 +  // algorithm
                   1 +  // label count
                   4 +  // original ttl
                   4 +  // expiration
                   4 +  // inception
                   2 +  // key tag
                   signer.size() +
                   signature.size();
        }

        virtual uint16_t type() const
        {
            return TYPE_RRSIG;
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };


    class RecordDNSKey : public ResourceData
    {
    private:
        uint16_t             flags;
        uint8_t              algorithm;
        std::vector<uint8_t> public_key;

    public:
        static const uint16_t SIGNED_KEY = 1 << 7;
        static const uint8_t  RSAMD5     = 1;
        static const uint8_t  RSASHA1    = 5;
        static const uint8_t  RSASHA256  = 8;
        static const uint8_t  RSASHA512  = 10;

        static const uint16_t KSK = 1 << 8;
        static const uint16_t ZSK = 0;

        RecordDNSKey( uint16_t f, uint8_t algo, const std::vector<uint8_t> key )
            : flags( f ), algorithm( algo ), public_key( key )
        {
        }

        virtual std::string toString() const;

        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t size() const
        {
            return 2 + 1 + 1 + public_key.size();
        }

        virtual uint16_t type() const
        {
            return TYPE_DNSKEY;
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    class RecordDS : public ResourceData
    {
    private:
        uint16_t             key_tag;
        uint8_t              algorithm;
        uint8_t              digest_type;
        std::vector<uint8_t> digest;

        RecordDS( uint16_t tag, uint8_t alg, uint8_t dtype, const std::vector<uint8_t> &d )
            : key_tag( tag ), algorithm( alg ), digest_type( dtype ), digest( d )
        {}

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t size() const
        {
            return 2 + 1 + 1 + digest.size();
        }

        virtual uint16_t type() const
        {
            return TYPE_DS;
        }

        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };


    const uint8_t PROTOCOL_TLS    = 0x01;
    const uint8_t PROTOCOL_MAIL   = 0x02;
    const uint8_t PROTOCOL_DNSSEC = 0x03;
    const uint8_t PROTOCOL_IPSEC  = 0x04;
    const uint8_t PROTOCOL_ANY    = 0xFF;

    const uint8_t ALGORITHM_DH = 0x02;

    class RecordKey : public ResourceData
    {
    public:
        uint8_t ac;
        uint8_t xt;
        uint8_t namtyp;
        uint8_t sig;

        uint8_t    protocol;
        uint8_t    algorithm;
        PacketData public_key;

    public:
        RecordKey( uint8_t in_ac        = 0,
                   uint8_t in_xt        = 0,
                   uint8_t in_namtyp    = 0,
                   uint8_t in_sig       = 0,
                   uint8_t in_protocol  = PROTOCOL_DNSSEC,
                   uint8_t in_algorithm = ALGORITHM_DH )
            : ac( 0 ), xt( 0 ), namtyp( 0 ), sig( 0 ), protocol( in_protocol ), algorithm( in_algorithm )
        {
        }

        virtual std::string toString() const
        {
            return "";
        }

        virtual void outputWireFormat( WireFormat &message )
        {
            message.pushUInt8( 0 );
            message.pushUInt8( 0 );
            message.pushUInt8( protocol );
            message.pushUInt8( algorithm );
        }
        virtual uint16_t size() const
        {
            return 4;
        }

        virtual uint16_t type() const
        {
            return TYPE_KEY;
        }
    };

    class OptPseudoRROption
    {
    public:
        virtual ~OptPseudoRROption()
        {
        }
        virtual std::string toString() const                       = 0;
        virtual void        outputWireFormat( WireFormat & ) const = 0;
        virtual uint16_t    code() const                           = 0;
        virtual uint16_t    size() const                           = 0;
    };

    typedef boost::shared_ptr<OptPseudoRROption> OptPseudoRROptPtr;

    class RAWOption : public OptPseudoRROption
    {
    private:
        uint16_t             option_code;
        uint16_t             option_size;
        std::vector<uint8_t> option_data;

    public:
        RAWOption( uint16_t in_code, uint16_t in_size, const std::vector<uint8_t> &in_data )
            : option_code( in_code ), option_size( in_size ), option_data( in_data )
        {
        }

        virtual std::string toString() const;
        virtual void        outputWireFormat( WireFormat & ) const;
        virtual uint16_t    code() const
        {
            return option_code;
        }
        virtual uint16_t size() const
        {
            return option_size;
        }
    };

    class NSIDOption : public OptPseudoRROption
    {
    private:
        std::string nsid;

    public:
        NSIDOption( const std::string &id = "" ) : nsid( id )
        {
        }

        virtual std::string toString() const
        {
            return "NSID: \"" + nsid + "\"";
        }
        virtual void     outputWireFormat( WireFormat & ) const;
        virtual uint16_t code() const
        {
            return OPT_NSID;
        }
        virtual uint16_t size() const
        {
            return 2 + 2 + nsid.size();
        }

        static OptPseudoRROptPtr parse( const uint8_t *begin, const uint8_t *end );
    };

    class ClientSubnetOption : public OptPseudoRROption
    {
    private:
        uint16_t    family;
        uint8_t     source_prefix;
        uint8_t     scope_prefix;
        std::string address;

        static unsigned int getAddressSize( uint8_t prefix );

    public:
        static const int IPv4 = 1;
        static const int IPv6 = 2;

        ClientSubnetOption( uint16_t fam, uint8_t source, uint8_t scope, const std::string &addr )
            : family( fam ), source_prefix( source ), scope_prefix( scope ), address( addr )
        {
        }

        virtual std::string toString() const;
        virtual void        outputWireFormat( WireFormat & ) const;
        virtual uint16_t    code() const
        {
            return OPT_CLIENT_SUBNET;
        }
        virtual uint16_t size() const;

        static OptPseudoRROptPtr parse( const uint8_t *begin, const uint8_t *end );
    };

    class RecordOptionsData : public ResourceData
    {
    public:
        std::vector<OptPseudoRROptPtr> options;

    public:
        RecordOptionsData( const std::vector<OptPseudoRROptPtr> &in_options = std::vector<OptPseudoRROptPtr>() )
            : options( in_options )
        {
        }

        virtual std::string toString() const;
        virtual void outputWireFormat( WireFormat &message ) const;
        virtual uint16_t type() const
        {
            return TYPE_OPT;
        }
        virtual uint16_t size() const;

        const std::vector<OptPseudoRROptPtr> &getOptions() const
        {
            return options;
        }
        static ResourceDataPtr parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end );
    };

    struct OptPseudoRecord {
        Domainname                      domainname;
        uint16_t                        payload_size;
        uint8_t                         rcode;
	bool                            dobit;
        boost::shared_ptr<ResourceData> record_options_data;
        uint32_t                        offset;

        OptPseudoRecord() : domainname( "." ), payload_size( 1280 ), rcode( 0 ), dobit(false), offset( NO_COMPRESSION )
        {
        }
    };

    class RecordTKey : public ResourceData
    {
    public:
        Domainname domain;
        Domainname algorithm;
        uint32_t   inception;
        uint32_t   expiration;
        uint16_t   mode;
        uint16_t   error;
        PacketData key;
        PacketData other_data;

    public:
        RecordTKey( const std::string &dom    = "",
                    const std::string &algo   = "HMAC-MD5.SIG-ALG.REG.INT",
                    uint32_t           incept = 0,
                    uint32_t           expire = 0,
                    uint16_t           m      = 0,
                    uint16_t           err    = 0,
                    PacketData         k      = PacketData(),
                    PacketData         other  = PacketData() )
            : domain( dom ), algorithm( algo ), inception( incept ), expiration( expire ), mode( m ), error( err ),
              key( k ), other_data( other )
        {
        }

        virtual std::string toString() const
        {
            return "";
        }
        virtual void     outputWireFormat( WireFormat & ) const;
        virtual uint16_t type() const
        {
            return TYPE_TKEY;
        }
        virtual uint16_t size() const;
    };

    struct TSIGInfo {
        std::string name;
        PacketData  key;
        std::string algorithm;
        PacketData  mac;
        uint64_t    signed_time;
        uint16_t    fudge;
        uint16_t    mac_size;
        uint16_t    original_id;
        uint16_t    error;
        PacketData  other;

        TSIGInfo()
            : name(), key(), algorithm( "HMAC-MD5.SIG-ALG.REG.INT" ), mac(), signed_time( 0 ), fudge( 0 ),
              mac_size( 0 ), original_id( 0 ), error( 0 ), other()
        {
        }
    };

    class RecordTSIGData : public ResourceData
    {
    public:
        Domainname key_name;
        Domainname algorithm;
        uint64_t   signed_time;
        uint16_t   fudge;
        uint16_t   mac_size;
        PacketData mac;
        uint16_t   original_id;
        uint16_t   error;
        uint16_t   other_length;
        PacketData other;

    public:
        RecordTSIGData( const std::string &in_key_name     = "",
                        const std::string &in_algo         = "HMAC-MD5.SIG-ALG.REG.INT",
                        uint64_t           in_signed_time  = 0,
                        uint16_t           in_fudge        = 0,
                        uint16_t           in_mac_size     = 0,
                        const PacketData & in_mac          = PacketData(),
                        uint16_t           in_original_id  = 0,
                        uint16_t           in_error        = 0,
                        uint16_t           in_other_length = 0,
                        const PacketData & in_other        = PacketData() )
            : key_name( in_key_name ), algorithm( in_algo ), signed_time( in_signed_time ), fudge( in_fudge ),
              mac_size( in_mac_size ), mac( in_mac ), original_id( in_original_id ), error( in_error ),
              other_length( in_other_length ), other( in_other )
        {
        }

        virtual std::string toString() const;
        virtual void        outputWireFormat( WireFormat & ) const;
        virtual uint16_t    type() const
        {
            return TYPE_TSIG;
        }
        virtual uint16_t size() const;

        static ResourceDataPtr
        parse( const uint8_t *packet, const uint8_t *begin, const uint8_t *end, const Domainname &key_name );
    };

    class RecordTSIG
    {
    public:
        Domainname name;
        Domainname algorithm;
        uint64_t   signed_time;
        uint16_t   fudge;
        uint16_t   mac_size;
        PacketData mac;
        uint16_t   original_id;
        uint16_t   error;
        uint16_t   other_length;
        PacketData other;

    public:
        RecordTSIG( const std::string &in_name         = "",
                    const std::string &in_algo         = "HMAC-MD5.SIG-ALG.REG.INT",
                    uint64_t           in_signed_time  = 0,
                    uint16_t           in_fudge        = 0,
                    uint16_t           in_mac_size     = 0,
                    PacketData         in_mac          = PacketData(),
                    uint16_t           in_original_id  = 0,
                    uint16_t           in_error        = 0,
                    uint16_t           in_other_length = 0,
                    PacketData         in_other        = PacketData() )
            : name( in_name ), algorithm( in_algo ), signed_time( in_signed_time ), fudge( in_fudge ),
              mac_size( in_mac_size ), mac( in_mac ), original_id( in_original_id ), error( in_error ),
              other_length( in_other_length ), other( in_other )
        {
        }
    };

    struct QuestionSectionEntry {
        Domainname q_domainname;
        uint16_t   q_type;
        uint16_t   q_class;
        uint16_t   q_offset;

        QuestionSectionEntry() : q_type( 0 ), q_class( 0 ), q_offset( NO_COMPRESSION )
        {
        }

	uint16_t size() const;
    };

    struct ResponseSectionEntry {
        Domainname      r_domainname;
        uint16_t        r_type;
        uint16_t        r_class;
        uint32_t        r_ttl;
        ResourceDataPtr r_resource_data;
        uint32_t        r_offset;

        ResponseSectionEntry() : r_type( 0 ), r_class( 0 ), r_ttl( 0 ), r_offset( NO_COMPRESSION )
        {
        }

    	uint16_t size() const;
    };

    struct QueryPacketInfo {
        uint16_t                          id;
        Opcode                            opcode;
        bool                              recursion;
        bool                              edns0;
        std::vector<QuestionSectionEntry> question;
        OptPseudoRecord                   opt_pseudo_rr;

        QueryPacketInfo( uint16_t                                 in_id        = 0,
                         Opcode                                   in_opcode    = OPCODE_QUERY,
                         bool                                     in_recursion = false,
                         bool                                     in_edns0     = false,
                         const std::vector<QuestionSectionEntry> &in_question  = std::vector<QuestionSectionEntry>(),
                         const OptPseudoRecord &                  in_opt_pseudo_rr = OptPseudoRecord() )
            : id( in_id ), opcode( in_opcode ), recursion( in_recursion ), edns0( in_edns0 ), question( in_question ),
              opt_pseudo_rr( in_opt_pseudo_rr )
        {
        }
    };

    struct ResponsePacketInfo {
        uint16_t id;
        uint16_t opcode;
        bool     recursion_available;
        bool     authoritative_answer;
        bool     truncation;
        bool     authentic_data;
        bool     checking_disabled;
        uint8_t  response_code;
        bool     edns0;

        OptPseudoRecord opt_pseudo_rr;

        std::vector<QuestionSectionEntry> question;
        std::vector<ResponseSectionEntry> answer;
        std::vector<ResponseSectionEntry> authority;
        std::vector<ResponseSectionEntry> additional_infomation;

        ResponsePacketInfo()
            : id( 0 ), opcode( 0 ), recursion_available( false ), authoritative_answer( false ), truncation( false ),
              authentic_data( false ), checking_disabled( false ), response_code( 0 ), edns0( false )
        {
        }
    };

    struct PacketInfo {
        uint16_t id;

        uint8_t query_response;
        uint8_t opcode;
        bool    authoritative_answer;
        bool    truncation;
        bool    recursion_desired;

        bool    recursion_available;
        bool    checking_disabled;
        bool    zero_field;
        bool    authentic_data;
        uint8_t response_code;

        bool edns0;
        bool tsig;

        OptPseudoRecord opt_pseudo_rr;
        RecordTSIGData  tsig_rr;

        std::vector<QuestionSectionEntry> question_section;
        std::vector<ResponseSectionEntry> answer_section;
        std::vector<ResponseSectionEntry> authority_section;
        std::vector<ResponseSectionEntry> additional_infomation_section;

        PacketInfo()
            : id( 0 ), query_response( 0 ), opcode( 0 ), authoritative_answer( 0 ), truncation( false ),
              recursion_desired( false ), recursion_available( false ), checking_disabled( false ), zero_field( 0 ),
              authentic_data( false ), response_code( 0 ), edns0( false ), tsig( false )
        {
        }
    };

    std::vector<uint8_t> generate_dns_packet( const PacketInfo &query );
    std::vector<uint8_t> generate_dns_query_packet( const QueryPacketInfo &query );
    std::vector<uint8_t> generate_dns_response_packet( const ResponsePacketInfo &response );
    void generate_dns_packet( const PacketInfo &query, WireFormat & );
    void generate_dns_query_packet( const QueryPacketInfo &query, WireFormat & );
    void generate_dns_response_packet( const ResponsePacketInfo &response, WireFormat & );
    PacketInfo parse_dns_packet( const uint8_t *begin, const uint8_t *end );
    QueryPacketInfo parse_dns_query_packet( const uint8_t *begin, const uint8_t *end );
    ResponsePacketInfo parse_dns_response_packet( const uint8_t *begin, const uint8_t *end );
    std::ostream &operator<<( std::ostream &os, const QueryPacketInfo &query );
    std::ostream &operator<<( std::ostream &os, const ResponsePacketInfo &response );
    std::ostream &print_header( std::ostream &os, const PacketInfo &packet );
    std::string type_code_to_string( Type t );
    std::string response_code_to_string( uint8_t rcode );

    struct PacketHeaderField {
        uint16_t id;

        uint8_t recursion_desired : 1;
        uint8_t truncation : 1;
        uint8_t authoritative_answer : 1;
        uint8_t opcode : 4;
        uint8_t query_response : 1;

        uint8_t response_code : 4;
        uint8_t checking_disabled : 1;
        uint8_t authentic_data : 1;
        uint8_t zero_field : 1;
        uint8_t recursion_available : 1;

        uint16_t question_count;
        uint16_t answer_count;
        uint16_t authority_count;
        uint16_t additional_infomation_count;
    };

    struct SOAField {
        uint32_t serial;
        uint32_t refresh;
        uint32_t retry;
        uint32_t expire;
        uint32_t minimum;
    };

    std::vector<uint8_t> convert_domainname_string_to_binary( const std::string &domainname,
                                                              uint32_t           compress_offset = NO_COMPRESSION );
    std::pair<std::string, const uint8_t *> convert_domainname_binary_to_string( const uint8_t *packet,
                                                                                 const uint8_t *domainame,
                                                                                 int recur = 0 ) throw( FormatError );
    std::vector<uint8_t> generate_question_section( const QuestionSectionEntry &q );
    std::vector<uint8_t> generate_response_section( const ResponseSectionEntry &r );
    void generate_question_section( const QuestionSectionEntry &q, WireFormat &message );
    void generate_response_section( const ResponseSectionEntry &r, WireFormat &message );

    typedef std::pair<QuestionSectionEntry, const uint8_t *> QuestionSectionEntryPair;
    typedef std::pair<ResponseSectionEntry, const uint8_t *> ResponseSectionEntryPair;
    QuestionSectionEntryPair parse_question_section( const uint8_t *packet, const uint8_t *section );
    ResponseSectionEntryPair parse_response_section( const uint8_t *packet, const uint8_t *section );

    ResponseSectionEntry generate_opt_pseudo_record( const OptPseudoRecord & );
    OptPseudoRecord      parse_opt_pseudo_record( const ResponseSectionEntry & );

    void
    addTSIGResourceRecord( const TSIGInfo &tsig_info, WireFormat &message, const PacketData &query_mac = PacketData() );
    bool
    verifyTSIGResourceRecord( const TSIGInfo &tsig_info, const PacketInfo &packet_info, const WireFormat &message );

    template <typename Type>
    uint8_t *set_bytes( Type v, uint8_t *pos )
    {
        *reinterpret_cast<Type *>( pos ) = v;
        return pos + sizeof( v );
    }

    template <typename Type>
    Type get_bytes( const uint8_t **pos )
    {
        Type v = *reinterpret_cast<const Type *>( *pos );
        *pos += sizeof( Type );
        return v;
    }
}

#endif
