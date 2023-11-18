#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <gtkmm.h>
#include <gtkmm/application.h>
#include "visuals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

using namespace Glib;
using namespace Gtk;

class MyWindow : public ApplicationWindow {
protected:
    vector<string>                      filenames;
    int                                 filepos;
    corestring                          folder;
    corestring                          str;
public:
    /* constructor */                   MyWindow();
    /* destructor */                   ~MyWindow();
    ustring                             getFilenameDialog( const char *title, FileChooserAction fileAction, const char *extension );
    void                                on_image_load_clicked();
    void                                on_image_save_clicked();
    void                                on_bin_file_load();
    void                                on_bin_file_save();
    void                                on_bin_file_save_with_preview();
    void                                readFile( ustring filename );
    void                                saveFile( ustring filename );
    Box                                 gVBox, gHBox, gHBox2;
    MyArea                              drawArea;
};

class ExampleApplication : public Application {
protected:
    /* constructor */                   ExampleApplication( const char * appName );
    /* destructor */                   ~ExampleApplication();

public:
    static RefPtr<ExampleApplication>   create( const char * appName );

protected:
    void                                on_startup() override;
    void                                on_activate() override;

private:
    void                                on_window_hide();
    void                                on_menu_file_quit();
    void                                on_menu_help_about();
    void                                on_show_checkerboard_clicked();

    RefPtr<MyWindow>                    myWindow;
    Glib::RefPtr<Gio::SimpleAction>     show_checkerboard_action;
};

#endif // WINDOW_H_INCLUDED
