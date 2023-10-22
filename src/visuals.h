#ifndef VISUALS_H_INCLUDED
#define VISUALS_H_INCLUDED

#include <vector>
#include <map>
#include <gtkmm.h>
#include "watchface.h"

using namespace Glib;
using namespace Gtk;
using namespace std;

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
    int                                 widget_width;
    int                                 widget_height;
    corestring                          str;
public:
    /* constructor */                   MyArea();
    virtual                            ~MyArea();
    void                                updateTypes();
    void                                setup( const char *filename );
    void                                write( const char *filename );
    void                                saveFile();
    int                                 itemTextToID( const char *name );
    void                                resetShift();
    bool                                on_draw(const Cairo::RefPtr<::Cairo::Context>& cr);
    void                                on_width_changed();
    void                                on_height_changed();
    void                                on_mouse_moved( mousePosition pos );
    void                                on_mouse_pressed( uint button, mousePosition pos );
    void                                on_mouse_released( uint button, mousePosition pos );
    void                                on_types_changed();
    void                                on_add_clicked();
    void                                on_del_clicked();
    bool                                on_timeout();
    int                                 shift;
    bool                                preview;
    bool                                debug;
    watchface                           binfile;
    image                               view;
    Entry                               gPosX;
    Entry                               gPosY;
    Label                               gShift;
    ComboBoxText                        gTypes, gNewTypes;
    Button                              gLoad, gSave;
    Button                              gAdd, gDel;
};

#endif // VISUALS_H_INCLUDED
