#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED

#include <string>
#include <memory>
#include <vector>
#include <gtkmm.h>
#include <gdkmm/pixbuf.h>

using namespace Glib;
using namespace std;

struct corestring : public string {
    /* constructor */                   corestring();
    /* constructor */                   corestring( const string &src );
    /* constructor */                   corestring( const char *src );
    /* constructor */                   corestring( const char src );
    void                                formatva( const char *format, va_list &arg_list );
    void                                format( const char *format, ... );
    long                                toLong();
    void                                operator += ( const char * append );
    /* cast operator */                 operator const char *();
};

struct image {
    vector<unsigned int> imgbuff;
    RefPtr<Gdk::Pixbuf> img;
    RefPtr<Gdk::Pixbuf> getFromMemory( const unsigned char *file_buffer, int w, int h );
};

unsigned int                            crc32b( unsigned char *message, int len );

#endif // BASE_H_INCLUDED
