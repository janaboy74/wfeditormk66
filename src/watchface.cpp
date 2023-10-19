#include "base.h"
#include "watchface.h"
#include <memory>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>

///////////////////////////////////////
item::item() {
///////////////////////////////////////
    type = 0;
    width = 0;
    pos = 0;
    height = 0;
    posx = 0;
    posy = 0;
    dummy1 = 0;
    imgcount = 0;
    dummy3 = 0;
}

///////////////////////////////////////
item::item( const item &other ) {
///////////////////////////////////////
    operator = ( other );
}

///////////////////////////////////////
void item::operator = ( const item &other ) {
///////////////////////////////////////
    type = other.type;
    width = other.width;
    pos = other.pos;
    height = other.height;
    posx = other.posx;
    posy = other.posy;
    dummy1 = other.dummy1;
    imgcount = other.imgcount;
    memcpy( &dummy2, other.dummy2, sizeof( dummy2 ));
    dummy3 = other.dummy3;
}

///////////////////////////////////////
imgitem::imgitem():item() {
///////////////////////////////////////
    count = 0;
}

///////////////////////////////////////
imgitem::imgitem( const item &other ) : item( other ), count(0), orig(0), rgb32(0) {
///////////////////////////////////////
}

///////////////////////////////////////
imgitem::imgitem( const imgitem &other ) : item( other ), count(other.count), orig(other.orig), rgb32(other.rgb32) {
///////////////////////////////////////
}

///////////////////////////////////////
void imgitem::operator = ( const imgitem &other ) {
///////////////////////////////////////
    item::operator = ( other );
    count = other.count;
    orig = other.orig;
    rgb32 = other.rgb32;
}

///////////////////////////////////////
void imgitem::toorig() {
///////////////////////////////////////
    orig = shared_ptr<unsigned short[]>(new unsigned short[count]);

    for( size_t i = 0; i < count; ++i ) {
        unsigned int color32 = rgb32[ i ];
        unsigned char *cols = (unsigned char *)&color32;
        unsigned int r = ( cols[0] >> 3 ) & 0x1f;
        unsigned int g = ( cols[1] >> 3 ) & 0x1f;
        unsigned int b = ( cols[2] >> 3 ) & 0x1f;
        unsigned short origcolor = ( r << 11 ) | ( g << 6 ) | b;
        unsigned char *swp = ( unsigned char * ) &origcolor, tmp = swp[ 0 ];
        swp[ 0 ] = swp[ 1 ];
        swp[ 1 ] = tmp;
        orig[ i ] = origcolor;
    }
}

///////////////////////////////////////
void imgitem::torgb32() {
///////////////////////////////////////
    rgb32 = shared_ptr<unsigned int[]>(new unsigned int[count]);

    for( size_t i = 0; i < count; ++i ) {
        unsigned short origcolor = orig[ i ];
        unsigned char *swp = ( unsigned char * ) &origcolor, tmp = swp[ 0 ];
        swp[ 0 ] = swp[ 1 ];
        swp[ 1 ] = tmp;
        unsigned int r = ((origcolor >> 11 ) & 0x1f ) << 3;
        unsigned int g = ((origcolor >> 6 ) & 0x1f ) << 3;
        unsigned int b = ((origcolor >> 0 ) & 0x1f ) << 3;
        unsigned int a = r+g+b ? 0xff : 0x00;
        unsigned char *cols = (unsigned char *)&rgb32[ i ];
        cols[ 0 ] = r;
        cols[ 1 ] = g;
        cols[ 2 ] = b;
        cols[ 3 ] = a;
    }
}

///////////////////////////////////////
watchface::watchface() : maxheight( 0 ) {}
///////////////////////////////////////

///////////////////////////////////////
bool watchface::hasitem( int itemid ) {
///////////////////////////////////////
    return items.find( itemid ) != items.end();
}

///////////////////////////////////////
bool watchface::readFile( const char *filename ) {
///////////////////////////////////////
    struct stat st;
    int fd = ::open( filename, O_RDONLY );
    fstat( fd, &st );
    size = st.st_size;
    buffer = shared_ptr<unsigned char[]>( new unsigned char[ size ]);
    size_t readBytes = read( fd, buffer.get(), size );
    close( fd );
    bool result = ( readBytes == size );
    curfilename = filename;
    parse();
    return result;
}

///////////////////////////////////////
void watchface::writeFile( const char * filename ) {
///////////////////////////////////////
    size_t len;
    (void) len;
    size_t pos = items.size() * sizeof( item ) + 2 + sizeof( header );

    for( auto &itm : items ) {
        itm.second.pos = pos;
        pos += itm.second.width * itm.second.height * itm.second.imgcount * 2;
    }

    int fd = ::open( filename, O_CREAT | O_TRUNC | O_WRONLY, 0777 );
    hdr.datasize = pos - sizeof( header );
    shared_ptr<unsigned char[]> writebuff;
    writebuff = shared_ptr<unsigned char[]>( new unsigned char[ pos - sizeof( hdr )]);
    size_t writepos = 0;

    for( auto &itm : items ) {
        memcpy( writebuff.get() + writepos, (item*)&itm.second, sizeof( item ));
        writepos += sizeof( item );
    }

    memset( writebuff.get() + writepos, 0, 2 );
    writepos += sizeof( short );

    for( auto &itm : items ) {
        int sz = itm.second.width * itm.second.height * itm.second.imgcount * 2;
        memcpy( writebuff.get() + writepos, itm.second.orig.get(), sz );
        writepos += sz;
    }

    hdr.crc32b = crc32b( writebuff.get(), hdr.datasize );
    len = ::write( fd, &hdr, sizeof( hdr ));
    len = ::write( fd, writebuff.get(), hdr.datasize );
    close( fd );
}

///////////////////////////////////////
void watchface::parse() {
///////////////////////////////////////
    unsigned char *start = ( unsigned char * ) buffer.get();
    hdr = *( header * ) start;
    item *tmp = (item *) ( start + sizeof( header ) );
    items.clear();

    for( ; tmp->type; ++tmp ) {
        if( tmp->type == 13 )
            tmp->imgcount = 14;

        if( tmp->type == 24 )
            tmp->imgcount = 8;

        items.insert(pair<int, imgitem>( tmp->type, *tmp ));
    }

    int shift = ((unsigned char*)tmp) - start;
    maxheight = 0;
    unsigned char *data = (unsigned char *)tmp;

    for( auto &item : items ) {
        item.second.pos = ( item.second.pos - shift );
        maxheight += item.second.height;
        item.second.count = item.second.width * item.second.height * item.second.imgcount;
        item.second.orig = shared_ptr<unsigned short[]>(new unsigned short[item.second.count]);
        memcpy( item.second.orig.get(), data + item.second.pos, item.second.count * 2 );
        item.second.torgb32();
    }
}
