#ifndef VISUALS_H_INCLUDED
#define VISUALS_H_INCLUDED

#include <vector>
#include <map>
#include <gtkmm.h>
#include <gtkmm/window.h>
#include <chrono>
#include "watchface.h"
#include <sigc++/sigc++.h>

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
    int                                 widgetWidth;
    int                                 widgetHeight;
    corestring                          str;
    time_point<system_clock>            referenceTime;
    using type_filename_changed = sigc::signal<void ( const char * )>;
    type_filename_changed               signal_filename_changed;
public:
    /* constructor */                   MyArea();
    virtual                            ~MyArea();
    void                                updateTypes();
    void                                setup( const char *filename );
    void                                setupDir( const char * directory );
    void                                write( const char *filename );
    void                                createPreview();
    corestring                          getDefault( int id );
    int                                 itemTextToID( const char *name );
    void                                limitShift();
    void                                resetShift();
    void                                initFields();
    void                                renderPreview( const Cairo::RefPtr<::Cairo::Context>& cr );
    bool                                on_draw( const Cairo::RefPtr<::Cairo::Context>& cr );
    void                                on_item_posX_changed();
    void                                on_item_posY_changed();
    void                                on_def_value_changed();
    bool                                on_window_key_pressed( GdkEventKey* event, Gtk::Widget *focus );
    void                                on_types_changed();
    void                                on_add_clicked();
    void                                on_del_clicked();
    void                                on_add_height_clicked();
    void                                on_copy_image_clicked();
    bool                                on_timeout();
    type_filename_changed               filename_changed();
    int                                 watchfaceWidth;
    int                                 watchfaceHeight;
    int                                 shift;
    bool                                preview;
    bool                                debug;
    uint                                button;
    watchface                           binfile;
    image                               view;
    Label                               gXText;
    Label                               gYText;
    RefPtr<Gtk::Adjustment>             aPosX, aPosY;
    SpinButton                          gPosXSpin, gPosYSpin;
    Button                              gLoad, gSave;
    Button                              gAdd, gDel;
    Entry                               gHeightFrame;
    Button                              gAddHeight;
    Label                               gDefValueText;
    Entry                               gDefValue;
    ComboBoxText                        gTypes, gNewTypes;
    CheckButton                         gCopyImage;
    image                               background;
    bool                                showCheckerboard;
    corestring                          folder;
    vector<string>                      filenames;
    int                                 filepos;
};

#endif // VISUALS_H_INCLUDED
