#include "base.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

///////////////////////////////////////
corestring::corestring() : string() {
///////////////////////////////////////
}

///////////////////////////////////////
corestring::corestring( const string &src ) : string( src ) {
///////////////////////////////////////
}

///////////////////////////////////////
corestring::corestring( const char *src ) : string( src ) {
///////////////////////////////////////
}

///////////////////////////////////////
corestring::corestring( const char src ) : string( &src, 1 ) {
///////////////////////////////////////
}

///////////////////////////////////////
void corestring::formatva( const char *format, va_list &arg_list ) {
///////////////////////////////////////
    if( format ) {
        va_list cova;
        va_copy( cova, arg_list );
        int size = vsnprintf( NULL, 0, format, cova );
        va_end( arg_list );
        resize( size );
        va_copy( cova, arg_list );
        vsnprintf( &at( 0 ), size + 1, format, cova );
        va_end( arg_list );
    }
}

///////////////////////////////////////
void corestring::format( const char *format, ... ) {
///////////////////////////////////////
    if( format ) {
        va_list arg_list;
        va_start( arg_list, format );
        formatva( format, arg_list );
        va_end( arg_list );
    }
}

///////////////////////////////////////
long corestring::toLong() {
///////////////////////////////////////
    return atol( c_str() );
}

///////////////////////////////////////
void corestring::operator += ( const char * append ) {
///////////////////////////////////////
    auto length = strlen( append );
    auto prevSize = size();
    resize( prevSize + length );
    strncpy(( char *) &*begin() + prevSize, append, length );
}

///////////////////////////////////////
corestring::operator const char *() {
///////////////////////////////////////
    return c_str();
}

///////////////////////////////////////
RefPtr<Gdk::Pixbuf> image::getFromMemory( const unsigned char *file_buffer, int w, int h ) {
///////////////////////////////////////
    img.reset();
    img = Gdk::Pixbuf::create_from_data((const guint8 *) file_buffer, Gdk::Colorspace::COLORSPACE_RGB, true, 8, w, h, w * 4 );
    return img;
}

///////////////////////////////////////
unsigned int crc32b(unsigned char *message, int len) {
///////////////////////////////////////
    int i, j;
    unsigned int byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;

    for( i = 0; i < len ; ++i) {
        byte = message[i];            // Get next byte.
        crc = crc ^ byte;

        for (j = 7; j >= 0; j--) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }

    return ~crc;
}
