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
watchface::watchface() : maxHeight( 0 ), exception( { 47, 48, 71 } ) {}
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
    memset(&hdr.compress[0], 0, sizeof(hdr.compress));// !!! remove if compress is done !!!

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
vector<uint8_t> watchface::compressline( vector<uint16_t> src ) {
///////////////////////////////////////
    vector<uint8_t> output;
    compress cprs;
    if( src.size() < 6 ) {
        cprs.count = 1;
        cprs.size = src.size();
        output.insert( output.end(), ( uint8_t *) &cprs, ( uint8_t *) ( &cprs + 1 ));
        output.insert( output.end(), ( uint8_t *) &src.at( 0 ), ( uint8_t *) &src.at( cprs.size ));
        return output;
    }
    list<match> found;
    match mach;
    uint32_t maxlen = src.size();
    uint32_t maxdiff = maxlen / 2;
    uint32_t acount = 0;
    int32_t limit = 1;
    if( maxdiff > 127 )
        maxdiff = 127;
    uint16_t col = src[ 0 ];
    vector<pair<uint32_t,uint16_t>> cnts;
    int32_t cnt = 0;
    for( uint32_t n = 1; n < maxlen; ++n ) {
        uint16_t acol = src[ n ];
        if( col == acol ) {
            ++cnt;
            continue;
        }
        cnts.push_back( pair<uint32_t,uint16_t>( cnt + 1, col ));
        col = acol;
        cnt = 0;
    }
    cnt = -1;
    uint32_t size = 0;
    uint32_t act = 0;
    uint32_t from = 0;
    int32_t acnt = 0;
    for( uint32_t s = 0; s < cnts.size(); ++s ) {
        acnt = cnts[ s ].first;
        if( cnt == acnt ) {
            ++size;
            continue;
        }
        ++size;
        if( cnt > 0 ) {
            mach.win = cnt * size - size;
            act = 0;
            for( uint32_t t = 0; t < from; ++t ) {
                uint32_t &tcnt = cnts[ t ].first;
                act += tcnt;
            }
            if( mach.win > limit ) {
                mach.count = cnt;
                mach.start = act;
                mach.block.clear();
                for( uint32_t n = 0; n < size; ++n ) {
                    mach.block.push_back( cnts[ from + n ].second );
                }
                found.push_back( mach );
            }
        }
        size = 0;
        from = s;
        cnt = acnt;
    }
    if( size > 0 ) {
        mach.win = cnt * size - size;
        act = 0;
        for( uint32_t t = 0; t < from; ++t ) {
            uint32_t &tcnt = cnts[ t ].first;
            act += tcnt;
        }
        if( mach.win > limit ) {
            mach.count = acnt;
            mach.start = act;
            mach.block.clear();
            for( uint32_t n = 0; n < size; ++n ) {
                mach.block.push_back( cnts[ from + n ].second );
            }
            found.push_back( mach );
        }
    }
    for( int32_t pos = 0; pos < maxlen; ) {
        auto mch = found.end();
        int32_t npos = maxlen;
        int32_t win = 0;
        for( auto next = found.begin(); next != found.end(); ++next ) {
            if( npos >= next->start && next->start >= pos ) {
                if( next->win > win ) {
                    mch = next;
                    npos = next->start;
                    win = next->win;
                }
            }
        }
        if( npos > pos ) {
            int32_t ps = 0;
            int32_t sz = npos - pos;
            for( ;; ) {
                cprs.count = 1;
                if( ps + 255 < sz ) {
                    cprs.size = 255;
                } else {
                    cprs.size = sz - ps;
                }
                if( ps >= sz )
                    break;
                output.insert( output.end(), ( uint8_t *) &cprs, ( uint8_t *) ( &cprs + 1 ));
                output.insert( output.end(), ( uint8_t *) &*( src.begin() + pos + ps), ( uint8_t *) &*( src.begin() + pos + ps + cprs.size ));
                ps += cprs.size;
            }
            npos = pos + ps;
        }
        if( found.end() == mch )
            break;
        if( mch != found.end() ) {
            int32_t pc = 0;
            for( ;; ) {
                cprs.size = mch->block.size();
                if( pc + 255 < mch->count ) {
                    cprs.count = 255;
                } else {
                    cprs.count = mch->count - pc;
                }
                if( pc >= mch->count )
                    break;
                output.insert( output.end(), ( uint8_t *) &cprs, ( uint8_t *) ( &cprs + 1 ));
                output.insert( output.end(), ( uint8_t *) &*mch->block.begin(), ( uint8_t *) &*mch->block.end() );
                pc += cprs.count;
            }
            npos = mch->start + mch->count * mch->block.size();
        }
        pos = npos;
    }
    return output;
}

///////////////////////////////////////
void watchface::writeCompressed( const char * filename ) {
///////////////////////////////////////
    size_t datasize = items.size() * sizeof( item ) + 2 + sizeof( header );
    map<int, packeditem> packedmap;

    for( auto &item : items ) {
        packeditem packed;
        auto &res = packed.data;
        uint32_t imgsize = item.second.width * item.second.height;
        item.second.toOrig();
        if((( item.first >= 4 && item.first <= 42 ) | exception.contains( item.first ))) {
            for( uint16_t imgid = 0; imgid < item.second.imgCount; ++imgid ) {
                vector<uint8_t> result;
                vector<uint32_t> ps;
                auto start = item.second.orig.get() + imgid * imgsize;
                for( uint32_t line = 0; line < item.second.height; ++line ) {
                    vector<uint16_t> src;
                    src.assign(( uint16_t * )( start + line * item.second.width ), ( uint16_t * )( start + ( line + 1 ) * item.second.width ));
                    vector<uint8_t> compdata = compressline( src );
                    ps.push_back( result.size() );
                    result.insert( result.cend(), compdata.begin(), compdata.end());
                }
                ps.insert( ps.begin(), result.size() );
                auto dta = ps.size() * 4 - 4;
                for( auto &ps : ps ) {
                    ps += dta;
                }
                auto posptr = (const char * ) &*ps.begin();
                res.insert( res.end(), posptr, posptr + ps.size() * 4 );
                res.insert( res.end(), result.begin(), result.end() );
            }
        } else {
            auto posptr = (const char * ) &*item.second.orig.get();
            res.insert( res.end(), posptr, posptr + item.second.count * 2 );
        }
        packed.image = item.second;
        packed.image.pos = datasize;
        datasize += res.size();
        packed.positions.push_back( datasize );
        packed.sizes.push_back( res.size() );
        packedmap.insert( pair<int, packeditem>( item.first, packed ));
    }

    header head = hdr;

    head.compress[ 0 ] = 0x02ff0100;
    head.compress[ 1 ] = 0xffffff15;

    int fdout = ::open( filename, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0666 );
    head.datasize = datasize - sizeof( header );
    vector<uint8_t> writebuff;
    uint8_t *dt = 0;
    for( auto &itm : packedmap ) {
        dt = ( uint8_t * )( item* ) &itm.second.image;
        writebuff.insert( writebuff.end(), dt, dt + sizeof( item ));
    }

    unsigned short clss = 0;
    dt = ( uint8_t * ) &clss;
    writebuff.insert( writebuff.end(), dt, dt + 2 );

    for( auto &itm : packedmap ) {
        writebuff.insert( writebuff.end(), itm.second.data.begin(), itm.second.data.end() );
    }

    head.crc32b = crc32b( writebuff.data(), head.datasize );
    std::ignore = ::write( fdout, &head, sizeof( head ));
    std::ignore = ::write( fdout, writebuff.data(), head.datasize );
    close( fdout );
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

    int hdrCompress = hdr.compress[0] & 0xFFFFFF00; /* mask compress format */

    for( auto &item : items ) {
        item.second.pos = ( item.second.pos - shift );
        item.second.count = item.second.width * item.second.height * item.second.imgCount;
        item.second.orig = shared_ptr<unsigned short[]>( new unsigned short[ item.second.count ]);
        vector<uint32_t> startPositions;
        startPositions.resize( item.second.height );
        auto ptr = ( uint16_t * ) &*item.second.orig.get();
        auto end = ptr;

        // If bin file's compress format(address 0x00000010~0x00000017) is 0x02FF01??.
        // We need to special process to get images.
        // ※ '?' is don't care.
        if( hdrCompress == COMPRESS_FORMAT && (( item.first >= 4 && item.first <= 42 ) | exception.contains( item.first ))) {
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
