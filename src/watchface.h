#ifndef WATCHFACE_H_INCLUDED
#define WATCHFACE_H_INCLUDED

#include <map>
#include "base.h"

using namespace std;

#define mapcontains( mapinst, id ) ( mapinst.find( id ) != mapinst.end() )

#pragma pack( push )
#pragma pack( 1 )

struct rgbColor {
    unsigned char                       r;
    unsigned char                       g;
    unsigned char                       b;
};

struct header {
    unsigned int                        d1;
    unsigned int                        datasize;
    unsigned int                        crc32b;
    unsigned short                      w;
    unsigned short                      h;
    int64_t                             filler;
};

struct item {
    unsigned short                      type;
    unsigned short                      width;
    unsigned int                        pos;
    unsigned short                      height;
    unsigned short                      posx;
    unsigned short                      posy;
    unsigned short                      dummy1;
    unsigned char                       imgcount;
    rgbColor                            alphakey;
    unsigned int                        dummy3;
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
};

struct watchface {
    size_t                              size;
    shared_ptr<unsigned char[]>         buffer;
    map<int, imgitem>                   items;
    header                              hdr;
    int                                 maxHeight;
    corestring                          curFilename;
    /* constructor */                   watchface();
    bool                                hasitem( int itemid );
    virtual bool                        readFile( const char *filename );
    void                                writeFile( const char * filename );
    void                                parse();
};

#endif // WATCHFACE_H_INCLUDED
