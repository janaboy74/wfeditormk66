#include <cstring>
#include <string>
#include <cstdarg>
#include <memory>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

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

//--------------------------------
template <typename I, typename V, typename Compare = std::less< I >, typename Alloc = std::allocator< std::pair< const I, V >>> struct coremap : public std::map< I, V > {
//--------------------------------
    coremap() : std::map< I, V > () {}
    coremap( std::initializer_list< std::pair< const I, V >> list, const Compare &cmp = Compare(), const Alloc &alloc = Alloc() ) : std::map< I, V >( list, cmp, alloc ) {}
    V &operator[]( const I item ) {
        static V dummy;
        if( contains( item ))
            return std::map< I, V >::operator[]( item );
        return dummy;
    }
    void insert( const I item, const V val ) {
        std::map< I, V >::insert( std::pair< I, V >( item, val ));
    }
    bool contains( const I item ) const {
        return this->find( item ) != this->end();
    }
};

set<uint8_t> exception = { 71 };
#define COMPRESS_FORMAT 0x02FF0100

#pragma pack( push )
#pragma pack( 1 )

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

///////////////////////////////////////
struct compress {
///////////////////////////////////////
    unsigned char size;
    unsigned char count;
};

#pragma pack( pop )

struct imgitem : public item {
    size_t                              count;
    shared_ptr<unsigned short[]>        orig;
    /* constructor */                   imgitem();
    /* constructor */                   imgitem( const item &other );
    /* constructor */                   imgitem( const imgitem &other );
    void                                operator = ( const imgitem &other );
    vector<uint32_t>                    positions;
    vector<uint32_t>                    sizes;
    vector<uint8_t>                     data;
};

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
imgitem::imgitem( const item &other ) : item( other ), count( 0 ), orig( 0 ) {
///////////////////////////////////////
}

///////////////////////////////////////
imgitem::imgitem( const imgitem &other ) : item( other ), count( other.count ), orig( other.orig ) {
///////////////////////////////////////
}

///////////////////////////////////////
void imgitem::operator = ( const imgitem &other ) {
///////////////////////////////////////
    item::operator = ( other );
    count = other.count;
    orig = other.orig;
}

map<int, imgitem> items;


struct cstr : public string {
  cstr()
    : string() {
  }

  cstr( const string &src )
    : string( src ) {
  }

  cstr( const char *src )
    : string( src ) {
  }

  cstr( const char src )
    : string( &src, 1 ) {
  }

  void formatva( const char *format, va_list &arg_list ) {
    if( format ) {
      va_list cova;
      va_copy( cova, arg_list );
      int size = vsnprintf( NULL, 0, format, cova );
      va_end( cova );
      resize( size );
      va_copy( cova, arg_list );
      vsnprintf( &at( 0 ), size + 1, format, cova );
      va_end( cova );
    }
  }

  void format( const char *format, ... ) {
    if( format ) {
      va_list arg_list;
      va_start( arg_list, format );
      formatva( format, arg_list );
      va_end( arg_list );
    }
  }
  operator const char *() {
    return c_str();
  }
};

struct match {
    uint32_t win;
    uint32_t start;
    uint32_t count;
    vector<uint16_t> block;
};

vector<uint8_t> compressline( vector<uint16_t> src ) {
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
unsigned int crc32b( unsigned char *message, int len ) {
///////////////////////////////////////
    int i, j;
    unsigned int byte, crc, mask;

    crc = 0xFFFFFFFF;

    for( i = 0; i < len ; ++i) {
        byte = message[ i ];            // Get next byte.
        crc = crc ^ byte;

        for( j = 7; j >= 0; j-- ) {     // Do eight times.
            mask = -( crc & 1 );
            crc = ( crc >> 1 ) ^ ( 0xEDB88320 & mask );
        }
    }

    return ~crc;
}

///////////////////////////////////////
void doCompress( const char *infile, const char *outfile, bool decompress ) {
///////////////////////////////////////
    ssize_t                              filesize;
    shared_ptr<unsigned char[]>         buffer;
    struct stat st;
    int fdin = ::open( infile, O_RDONLY | O_BINARY );
    if( fdin < 0 )
        return;
    fstat( fdin, &st );
    filesize = st.st_size;
    buffer = shared_ptr<unsigned char[]>( new unsigned char[ filesize ]);
    std::ignore = read( fdin, buffer.get(), filesize );
    close( fdin );
    unsigned char *start = ( unsigned char * ) buffer.get();
    header hdr = *( header * ) start;
    header head = hdr;
    item *tmp = ( item * ) ( start + sizeof( header ));
    items.clear();

    for( ; tmp->type; ++tmp ) {
        items.insert(pair<int, imgitem>( tmp->type, *tmp ));
    }

    unsigned char *data = ( unsigned char * ) tmp;
    int shift = data - start;

    coreset<uint8_t> exception = { 47, 48, 71 };

    int hdrCompress = hdr.compress[0] & 0xFFFFFF00; /* mask compress format */

    for( auto &item : items ) {
        item.second.pos = ( item.second.pos - shift );
        item.second.count = item.second.width * item.second.height * item.second.imgCount;
        item.second.orig = shared_ptr<unsigned short[]>( new unsigned short[ item.second.count ]);
        auto ptr = ( uint16_t * ) &*item.second.orig.get();
        auto end = ptr;

        if( hdrCompress == COMPRESS_FORMAT && (( item.first >= 4 && item.first <= 42 ) | exception.contains( item.first ))) {
            uint32_t *head = ( uint32_t * ) ( data + item.second.pos );
            vector<uint32_t> startPositions;
            startPositions.resize( item.second.height );
            vector<uint8_t> res;
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
                        compress *params = (compress*)src;
                        ++src;
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
        uint32_t filepos = 0;
        vector<uint8_t> res;
        uint32_t imgsize = item.second.width * item.second.height;
        if( !decompress && (( item.first >= 4 && item.first <= 42 ) | exception.contains( item.first ))) {
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
        item.second.positions.clear();
        item.second.positions.push_back( filepos );
        item.second.sizes.clear();
        item.second.sizes.push_back( res.size() );
        item.second.data = res;
    }

    size_t datasize = items.size() * sizeof( item ) + 2 + sizeof( header );

    for( auto &itm : items ) {
        itm.second.pos = datasize;
        datasize += itm.second.data.size();
    }

    if( decompress ) {
        printf( "%s decompression: %ld -> %ld\n", infile, filesize, datasize );
    } else {
        if( datasize > filesize ) {
            printf( "unable to compress: %s - the compressed size is larger than the original: %ld -> %ld\n", infile, filesize, datasize );
            return;
        } else {
            printf( "%s compression: %1.2f%%( %ld -> %ld )\n", infile, 100.f * ( filesize - datasize ) / filesize, filesize, datasize );
        }
    }

    if( decompress ) {
        head.compress[ 0 ] = 0x0;
        head.compress[ 1 ] = 0x0;
    } else {
        head.compress[ 0 ] = 0x02ff0100;
        head.compress[ 1 ] = 0xffffff15;
    }
    int fdout = ::open( outfile, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0666 );
    head.datasize = datasize - sizeof( header );
    vector<uint8_t> writebuff;
    uint8_t *dt = 0;
    for( auto &itm : items ) {
        dt = ( uint8_t * )( item* ) &itm.second;
        writebuff.insert( writebuff.end(), dt, dt + sizeof( item ));
    }

    unsigned short clss = 0;
    dt = ( uint8_t * ) &clss;
    writebuff.insert( writebuff.end(), dt, dt + 2 );

    for( auto &itm : items ) {
        writebuff.insert( writebuff.end(), itm.second.data.begin(), itm.second.data.end() );
    }

    head.crc32b = crc32b( writebuff.data(), head.datasize );
    std::ignore = ::write( fdout, &head, sizeof( head ));
    std::ignore = ::write( fdout, writebuff.data(), head.datasize );
    close( fdout );
}

int main( int argc, const char **argv )
{
    bool used = false;
    bool decompress = false;

    for( int i = 1; i < argc; ++i ) {
        if( !strcmp( argv[ i ], "-d" )) {
            decompress = true;
            continue;
        }
        if( argc >= i + 2 ) {
            const char *infile = argv[ i ];
            const char *outfile = argv[ ++i ]; ++i;
            doCompress( infile, outfile, decompress );
            used = true;
        }
    }
    if( !used ) {
        printf( "<<< Watchface compressor >>>\n" );
        printf( "usage:\n" );
        printf( "%s [input file] [output file ]\n", argv[ 0 ]);
    }

    return 0;
}
