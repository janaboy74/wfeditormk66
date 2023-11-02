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

struct mousePosition {
    double                              x;
    double                              y;
};

extern map<char, int>                   chrtopos;
extern vector<int>                      format;
extern vector<int>                      imgs;
extern map<int, int>                    defimgcount;

class MyArea : public Layout {
    int                                 posX, posY;
    mousePosition                       mousePressPosition;
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
    void                                resetShift();
    void                                renderPreview( const Cairo::RefPtr<::Cairo::Context>& cr );
    bool                                on_draw( const Cairo::RefPtr<::Cairo::Context>& cr );
    void                                on_width_changed();
    void                                on_height_changed();
    void                                on_def_value_changed();
    void                                on_mouse_moved( mousePosition pos );
    void                                on_mouse_pressed( uint button, mousePosition pos );
    void                                on_mouse_released( uint button, mousePosition pos );
    void                                on_types_changed();
    void                                on_add_clicked();
    void                                on_del_clicked();
    void                                on_add_height_clicked();
    bool                                on_timeout();
    int                                 shift;
    bool                                preview;
    bool                                debug;
    watchface                           binfile;
    image                               view;
    Entry                               gPosX;
    Entry                               gPosY;
    Label                               gShift;
    Button                              gLoad, gSave;
    Button                              gAdd, gDel;
    Entry                               gHeightFrame;
    Button                              gAddHeight;
    Entry                               gDefvalue;
    ComboBoxText                        gTypes, gNewTypes;
    image                               mask;
};

#endif // VISUALS_H_INCLUDED
