#include "base.h"
#include "watchface.h"
#include <memory>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __linux
#define O_BINARY 0
#endif

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

///////////////////////////////////////
RGBColor::RGBColor( unsigned char r, unsigned char g, unsigned char b ) {
///////////////////////////////////////
    this->r = r;
    this->g = g;
    this->b = b;
}

///////////////////////////////////////
unsigned short RGBColor::toPacked() const {
///////////////////////////////////////
    unsigned short origcolor = (( r >> 3 & 0x1f ) << 11 ) | (( g >> 3 & 0x1f ) << 6 ) | ( b >> 3 & 0x1f );
    unsigned char *color = ( unsigned char * ) &origcolor;
    swap( color[ 0 ], color[ 1 ]);
    return origcolor;
}

///////////////////////////////////////
void RGBColor::fromPacked( unsigned short packedColor ) {
///////////////////////////////////////
    unsigned char *color = ( unsigned char * ) &packedColor;
    swap( color[ 0 ], color[ 1 ]);
    r = (( packedColor >> 11 ) & 0x1f ) << 3;
    g = (( packedColor >> 6 ) & 0x1f ) << 3;
    b = (( packedColor >> 0 ) & 0x1f ) << 3;
}

///////////////////////////////////////
bool RGBColor::isBlack() const {
///////////////////////////////////////
    return !( r + g + b );
}

///////////////////////////////////////
item::item() {
///////////////////////////////////////
    type = 0;
    width = 0;
    pos = 0;
    height = 0;
    posX = 0;
    posY = 0;
    dummy1 = 0;
    imgCount = 0;
    copyImage = 0;
    dummy2 = 0;
    memset(clockHandsInfo, 0, sizeof(clockHandsInfo));
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
    posX = other.posX;
    posY = other.posY;
    dummy1 = other.dummy1;
    imgCount = other.imgCount;
    copyImage = other.copyImage;
    dummy2 = other.dummy2;
    memcpy(clockHandsInfo, other.clockHandsInfo, sizeof(clockHandsInfo));
}

///////////////////////////////////////
imgitem::imgitem():item() {
///////////////////////////////////////
    count = 0;
}

///////////////////////////////////////
imgitem::imgitem( const item &other ) : item( other ), count( 0 ), orig( 0 ), RGB32( 0 ) {
///////////////////////////////////////
}

///////////////////////////////////////
imgitem::imgitem( const imgitem &other ) : item( other ), count( other.count ), orig( other.orig ), RGB32( other.RGB32 ) {
///////////////////////////////////////
}

///////////////////////////////////////
void imgitem::operator = ( const imgitem &other ) {
///////////////////////////////////////
    item::operator = ( other );
    count = other.count;
    orig = other.orig;
    RGB32 = other.RGB32;
}

///////////////////////////////////////
void imgitem::toOrig() {
///////////////////////////////////////
    orig = shared_ptr<unsigned short[]>( new unsigned short[ count ]);

    for( size_t i = 0; i < count; ++i ) {
        unsigned int color32 = RGB32[ i ];
        const RGBColor &color = *( RGBColor *) &color32;
        unsigned short origcolor = color.toPacked();
        orig[ i ] = origcolor;
    }
}

///////////////////////////////////////
void imgitem::toRGB32() {
///////////////////////////////////////
    RGB32 = shared_ptr<unsigned int[]>( new unsigned int[ count ]);

    for( size_t i = 0; i < count; ++i ) {
        RGBColor color;
        color.fromPacked( orig[ i ]);
        unsigned char *cols = ( unsigned char * ) &RGB32[ i ];
        cols[ 0 ] = color.r;
        cols[ 1 ] = color.g;
        cols[ 2 ] = color.b;
        cols[ 3 ] = ( copyImage || !color.isBlack() ) ? 0xff : 0x00;
    }
}

///////////////////////////////////////
void imgitem::updateAlpha() {
///////////////////////////////////////
    for( size_t i = 0; i < count; ++i ) {
        RGBColor color;
        color.fromPacked( orig[ i ]);
        unsigned char *cols = ( unsigned char * ) &RGB32[ i ];
        cols[ 3 ] = ( copyImage || !color.isBlack() ) ? 0xff : 0x00;
    }
}

///////////////////////////////////////
watchface::watchface() : maxHeight( 0 ) {}
///////////////////////////////////////

///////////////////////////////////////
bool watchface::hasitem( int itemid ) {
///////////////////////////////////////
    return mapcontains( items, itemid );
}

///////////////////////////////////////
bool watchface::readFile( const char *filename ) {
///////////////////////////////////////
    struct stat st;
    int fd = ::open( filename, O_RDONLY | O_BINARY );
    if( fd < 0 )
        return false;
    fstat( fd, &st );
    size = st.st_size;
    buffer = shared_ptr<unsigned char[]>( new unsigned char[ size ]);
    size_t readBytes = read( fd, buffer.get(), size );
    close( fd );
    bool result = ( readBytes == size );
    curFilename = filename;
    parse();
    return result;
}

///////////////////////////////////////
void watchface::writeFile( const char * filename ) {
///////////////////////////////////////
    size_t len;
    (void) len;
    size_t pos = items.size() * sizeof( item ) + 2 + sizeof( header );
    hdr.compress = 0; // !!! remove if compress is done !!!

    for( auto &itm : items ) {
        itm.second.pos = pos;
        pos += itm.second.width * itm.second.height * itm.second.imgCount * 2;
    }

    int fd = ::open( filename, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0666 );
    hdr.datasize = pos - sizeof( header );
    shared_ptr<unsigned char[]> writebuff;
    writebuff = shared_ptr<unsigned char[]>( new unsigned char[ pos - sizeof( hdr )]);
    size_t writepos = 0;

    for( auto &itm : items ) {
        memcpy( writebuff.get() + writepos, ( item* ) &itm.second, sizeof( item ));
        writepos += sizeof( item );
    }

    memset( writebuff.get() + writepos, 0, 2 );
    writepos += sizeof( short );

    for( auto &itm : items ) {
        int sz = itm.second.width * itm.second.height * itm.second.imgCount * 2;
        memcpy( writebuff.get() + writepos, itm.second.orig.get(), sz );
        writepos += sz;
    }

    hdr.crc32b = crc32b( writebuff.get(), hdr.datasize );
    len = ::write( fd, &hdr, sizeof( hdr ));
    len = ::write( fd, writebuff.get(), hdr.datasize );
    close( fd );
}

///////////////////////////////////////
struct compress {
///////////////////////////////////////
    unsigned char size;
    unsigned char count;
};

///////////////////////////////////////
void watchface::parse() {
///////////////////////////////////////
    unsigned char *start = ( unsigned char * ) buffer.get();
    hdr = *( header * ) start;
    item *tmp = ( item * ) ( start + sizeof( header ));
    items.clear();

    for( ; tmp->type; ++tmp ) {
        items.insert(pair<int, imgitem>( tmp->type, *tmp ));
    }

    unsigned char *data = ( unsigned char * ) tmp;
    int shift = data - start;

    coreset<uint8_t> exception = { 71 };

    for( auto &item : items ) {
        item.second.pos = ( item.second.pos - shift );
        item.second.count = item.second.width * item.second.height * item.second.imgCount;
        item.second.orig = shared_ptr<unsigned short[]>( new unsigned short[ item.second.count ]);
        vector<uint32_t> startPositions;
        startPositions.resize( item.second.height );
        auto ptr = ( uint16_t * ) &*item.second.orig.get();
        auto end = ptr;
        if( hdr.compress == ~((uint64_t)0xfd00fefful) && (( item.first > 3 && item.first < 43 ) | exception.contains( item.first ))) { // 0xffffffff02ff0100
            uint32_t *head = ( uint32_t * ) ( data + item.second.pos );
            for( size_t imgID = 0 ; imgID < item.second.imgCount; ++imgID ) {
                unsigned char *start = ( unsigned char * ) head;
                uint32_t *next = ( uint32_t * ) ( start + *head ); ++head;
                start = ( unsigned char * ) head;
                memcpy(( void* ) &startPositions.at( 0 ), head, item.second.height * 4 );
                for( size_t y = 0; y < item.second.height; ++y ) {
                    ptr = end;
                    end = ( ptr + item.second.width );
                    unsigned short *src = ( unsigned short * ) ( start + startPositions[ y ] );
                    while( ptr < end ) {
                        compress *params = (compress*)src; ++src;
                        auto from = src;
                        for( size_t s = 0; s < params->size; ++s ) {
                            for( size_t c = 0; c < params->count; ++c ) {
                                if( ptr >= end )
                                    break;
                                *ptr = *from; ++ptr;
                            }
                            ++from;
                        }
                        src += params->size;
                    }
                }
                head = ++next;
            }
        } else {
            memcpy( item.second.orig.get(), data + item.second.pos, item.second.count * 2 );
        }
        item.second.toRGB32();
    }
    updateMaxHeight();
}

///////////////////////////////////////
void watchface::updateMaxHeight() {
///////////////////////////////////////
    maxHeight = 0;

    for( auto &item : items ) {
        maxHeight += item.second.height;
    }
}
