#ifndef WATCHFACE_H_INCLUDED
#define WATCHFACE_H_INCLUDED

#include <map>
#include "base.h"

using namespace std;

#pragma pack( push )
#pragma pack( 1 )

struct header {
    unsigned int                        d1;
    unsigned int                        datasize;
    unsigned int                        crc32b;
    unsigned short                      w;
    unsigned short                      h;
    unsigned char                       p1;
    unsigned char                       p2;
    unsigned char                       p3;
    unsigned char                       p4;
    unsigned int                        ffff;
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
    unsigned char                       dummy2[3];
    unsigned int                        dummy3;
    /* constructor */                   item();
    /* constructor */                   item( const item &other );
    void operator = ( const item &other );
};
#pragma pack( pop )

struct imgitem : public item {
    size_t                              count;
    shared_ptr<unsigned short[]>        orig;
    shared_ptr<unsigned int[]>          rgb32;
    /* constructor */                   imgitem();
    /* constructor */                   imgitem( const item &other );
    /* constructor */                   imgitem( const imgitem &other );
    void                                operator = ( const imgitem &other );
    void                                toorig();
    void                                torgb32();
};

struct watchface {
    size_t                              size;
    shared_ptr<unsigned char[]>         buffer;
    map<int, imgitem>                   items;
    header                              hdr;
    int                                 maxheight;
    corestring                          curfilename;
    /* constructor */                   watchface();
    bool                                hasitem( int itemid );
    virtual bool                        readFile( const char *filename );
    void                                writeFile( const char * filename );
    void                                parse();
};

#endif // WATCHFACE_H_INCLUDED
