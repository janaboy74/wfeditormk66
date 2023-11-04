#ifndef VISUALS_H_INCLUDED
#define VISUALS_H_INCLUDED

#include <vector>
#include <map>
#include <gtkmm.h>
#include <chrono>
#include "watchface.h"

using namespace Glib;
using namespace Gtk;
using namespace std;
using namespace std::chrono;

#ifndef uint
typedef unsigned int uint;
#endif

struct Point {
    double                              x;
    double                              y;
    /* constructor */                   Point( double x = 0, double y = 0 );
    /* constructor */                   Point( const Point &other );
    Point &operator                     = ( const Point &delta );
    Point operator                      - ( const Point &delta );
    Point operator                      + ( const Point &delta );
    double                              dist() const;
};

extern map<char, int>                   chrtopos;
extern vector<int>                      format;
extern vector<int>                      imgs;

class MyArea : public DrawingArea {
    int                                 posX, posY;
    Point                               pressPosition;
    bool                                buttonPressed;
    int                                 widgetWidth;
    int                                 widgetHeight;
    corestring                          str;
    time_point<system_clock>            referenceTime;
public:
    /* constructor */                   MyArea();
    virtual                            ~MyArea();
    void                                updateTypes();
    void                                setup( const char *filename );
    void                                write( const char *filename );
    void                                createPreview();
    corestring                          getDefault( int id );
    int                                 itemTextToID( const char *name );
    void                                limitShift();
    void                                resetShift();
    void                                initFields();
    void                                renderPreview( const Cairo::RefPtr<::Cairo::Context>& cr );
    bool                                on_draw( const Cairo::RefPtr<::Cairo::Context>& cr );
    void                                on_width_changed();
    void                                on_height_changed();
    void                                on_def_value_changed();
    void                                on_mouse_moved( const Point &pos );
    void                                on_mouse_pressed( uint button, const Point &pos );
    void                                on_mouse_released( uint button, const Point &pos );
    void                                on_types_changed();
    void                                on_add_clicked();
    void                                on_del_clicked();
    void                                on_add_height_clicked();
    void                                on_copy_image_clicked();
    bool                                on_timeout();
    int                                 watchfaceWidth;
    int                                 watchfaceHeight;
    int                                 shift;
    bool                                preview;
    bool                                debug;
    watchface                           binfile;
    image                               view;
    Label                               gXText;
    Label                               gYText;
    Entry                               gPosX;
    Entry                               gPosY;
    Button                              gLoad, gSave;
    Button                              gAdd, gDel;
    Entry                               gHeightFrame;
    Button                              gAddHeight;
    Label                               gDefValueText;
    Entry                               gDefValue;
    ComboBoxText                        gTypes, gNewTypes;
    CheckButton                         gCopyImage;
    image                               mask;
    image                               background;
    bool                                showCheckboard;
};

#endif // VISUALS_H_INCLUDED
