#ifndef WATCHFACE_H_INCLUDED
#define WATCHFACE_H_INCLUDED

#include <map>
#include "base.h"

using namespace std;

#define mapcontains( mapinst, id ) ( mapinst.find( id ) != mapinst.end() )
#define COMPRESS_FORMAT 0x02FF0100

///////////////////////////////////////
template <class V, typename Compare = std::less<V>, typename Alloc = std::allocator<V>> struct coreset : public std::set<V> {
///////////////////////////////////////
    coreset( std::initializer_list<V> init, const Compare& comp = Compare(), const Alloc& alloc = Alloc() ) : std::set<V>( init, comp, alloc ) {}
    V &operator[]( const V val ) {
        static V dummy;
        if( contains( val ))
            return std::set< V>::operator[]( val );
        return dummy;
    }
    bool contains( const V &val ) const {
        return std::set<V>::find( val ) != this->end();
    }
};

#pragma pack( push )
#pragma pack( 1 )

struct RGBColor {
    unsigned char                       r;
    unsigned char                       g;
    unsigned char                       b;
    /* constructor */                   RGBColor( unsigned char r = 0, unsigned char g = 0, unsigned char b = 0 );
    unsigned short                      toPacked() const;
    void                                fromPacked( unsigned short packedColor );
    bool                                isBlack() const;
};

struct header {
    unsigned int                        d1;
    unsigned int                        datasize;
    unsigned int                        crc32b;
    unsigned short                      w;
    unsigned short                      h;
    unsigned int                        compress[2];
};

struct item {
    unsigned short                      type;
    unsigned short                      width;
    unsigned int                        pos;
    unsigned short                      height;
    unsigned short                      posX;
    unsigned short                      posY;
    unsigned short                      dummy1;
    unsigned char                       imgCount;
    unsigned char                       copyImage;
    unsigned short                      dummy2;
    unsigned char                       clockHandsInfo[4];
    /* constructor */                   item();
    /* constructor */                   item( const item &other );
    void operator = ( const item &other );
};

#pragma pack( pop )

struct imgitem : public item {
    size_t                              count;
    shared_ptr<unsigned short[]>        orig;
    shared_ptr<unsigned int[]>          RGB32;
    /* constructor */                   imgitem();
    /* constructor */                   imgitem( const item &other );
    /* constructor */                   imgitem( const imgitem &other );
    void                                operator = ( const imgitem &other );
    void                                toOrig();
    void                                toRGB32();
    void                                updateAlpha();
};

///////////////////////////////////////
struct watchface {
///////////////////////////////////////
    struct compress {
        unsigned char size;
        unsigned char count;
    };
    struct match {
        uint32_t win;
        uint32_t start;
        uint32_t count;
        vector<uint16_t> block;
    };
    struct packeditem {
        item                                image;
        vector<uint32_t>                    positions;
        vector<uint32_t>                    sizes;
        vector<uint8_t>                     data;
    };
    size_t                              size;
    shared_ptr<unsigned char[]>         buffer;
    map<int, imgitem>                   items;
    vector<uint8_t>                     data;
    header                              hdr;
    coreset<uint8_t>                    exception;
    int                                 maxHeight;
    corestring                          curFilename;
    /* constructor */                   watchface();
    bool                                hasitem( int itemid );
    virtual bool                        readFile( const char *filename );
    void                                writeFile( const char * filename );
    vector<uint8_t>                     compressline( vector<uint16_t> src );
    void                                writeCompressed( const char * filename );
    void                                parse();
    void                                updateMaxHeight();
};

#endif // WATCHFACE_H_INCLUDED
