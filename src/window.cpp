#include "window.h"

///////////////////////////////////////
MyWindow::MyWindow() : filepos( 0 ), gVBox( ORIENTATION_VERTICAL ), gHBox( ORIENTATION_HORIZONTAL ), gHBox2( ORIENTATION_HORIZONTAL ) {
///////////////////////////////////////
    auto buffer = getcwd( nullptr, 0 );
    folder = buffer;
    folder += "/Watchfaces/";
    free( buffer );

#ifdef linux
    mkdir( folder.c_str(), 0777 );
#else
    mkdir( folder.c_str() );
#endif

    DIR *dir = opendir( folder.c_str() );
    struct dirent *dp;

    while (( dp = readdir( dir )) != nullptr ) {
        string filename = dp->d_name;

        if( filename.substr( filename.find_last_of( "." ) + 1 ) == "bin" )
            filenames.push_back( filename );
    }

    closedir( dir );
    corestring file;
    if( filenames.size() ) {
        file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str());
        readFile( file.c_str() );
    }
    set_default_size( 1000, 800 );
    add( gVBox );
    gHBox.add( drawArea.gTypes );
    drawArea.gLoad.set_label( "load" );
    gHBox.add( drawArea.gLoad );
    drawArea.gSave.set_label( "save" );
    gHBox.add( drawArea.gSave );
    gHBox.add( drawArea.gNewTypes );
    drawArea.gAdd.set_label( "add" );
    gHBox.add( drawArea.gAdd );
    drawArea.gDel.set_label( "del" );
    gHBox.add( drawArea.gDel );
    drawArea.gLoad.signal_clicked().connect( sigc::mem_fun( this, &MyWindow::on_image_load_clicked ));
    drawArea.gSave.signal_clicked().connect( sigc::mem_fun( this, &MyWindow::on_image_save_clicked ));
    drawArea.gTypes.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_types_changed ));
    drawArea.gAdd.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_add_clicked ));
    drawArea.gDel.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_del_clicked ));
    drawArea.set_can_focus();

    drawArea.gXText.set_label( "X" );
    gHBox.add( drawArea.gXText );
    gHBox.add( drawArea.gPosX );
    drawArea.gYText.set_label( "Y" );
    gHBox.add( drawArea.gYText );
    gHBox.add( drawArea.gPosY );
    drawArea.gPosX.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_width_changed ));
    drawArea.gPosY.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_height_changed ));

    gHBox.set_halign( Gtk::Align::ALIGN_START );
    gHBox2.set_halign( Gtk::Align::ALIGN_START );
    drawArea.set_vexpand( true );
    gVBox.add( gHBox );
    drawArea.gHeightFrame.set_text( "0" );
    gHBox2.add( drawArea.gHeightFrame );
    drawArea.gAddHeight.set_label( "add height" );
    gHBox2.add( drawArea.gAddHeight );
    gHBox2.add( drawArea.gDefvalue );
    drawArea.gCopyImage.set_label( "copy image" );
    gHBox2.add( drawArea.gCopyImage );
    drawArea.gAddHeight.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_add_height_clicked ));
    drawArea.gDefvalue.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_def_value_changed ));
    drawArea.gCopyImage.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_copy_image_clicked ));
    gVBox.add( gHBox2 );
    gVBox.add( drawArea );
    signal_timeout().connect( sigc::mem_fun( drawArea, &MyArea::on_timeout), 20 );
    drawArea.signal_draw().connect( sigc::mem_fun( drawArea, &MyArea::on_draw ));
    show_all_children();
    drawArea.grab_focus();
    signal_key_press_event().connect([&]( GdkEventKey* event )->bool {
        if( filenames.size() ) {
            if( GDK_KEY_q == event->keyval ) {
                if( --filepos < 0 )
                    filepos = filenames.size() - 1;

                if(( int ) filenames.size() > filepos ) {
                    corestring file;
                    file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str());
                    readFile( file.c_str() );
                    drawArea.resetShift();
                }
            }
            if( GDK_KEY_w == event->keyval ) {
                if( ++filepos >= ( int ) filenames.size() )
                    filepos = 0;

                if(( int ) filenames.size() > filepos ) {
                    corestring file;
                    file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str() );
                    readFile( file.c_str() );
                    drawArea.resetShift();
                }
            }
        }
        if( GDK_KEY_Escape == event->keyval )
            drawArea.initFields();
        if( GDK_KEY_F1 == event->keyval ) {
            drawArea.preview = true;
            drawArea.initFields();
        }
        if( GDK_KEY_F2 == event->keyval ) {
            drawArea.preview = false;
            drawArea.initFields();
        }
        if(( GDK_KEY_0 > event->keyval || GDK_KEY_9 < event->keyval ) && GDK_KEY_Left != event->keyval && GDK_KEY_Right != event->keyval &&
             GDK_KEY_BackSpace != event->keyval && GDK_KEY_Delete != event->keyval && GDK_KEY_End != event->keyval && GDK_KEY_Home != event->keyval &&
             ( get_focus() != &drawArea.gHeightFrame || GDK_KEY_minus != event->keyval )) {
            if( dynamic_cast<Gtk::Entry*>( get_focus() )) {
                return true;
            }
        }
        return false;
    }, false );
};

///////////////////////////////////////
MyWindow::~MyWindow() {};
///////////////////////////////////////

///////////////////////////////////////
ustring MyWindow::getFilenameDialog( const char *title, FileChooserAction fileAction, const char *extension ) {
///////////////////////////////////////
    auto dialog = new FileChooserDialog( title, fileAction );
    dialog->set_transient_for( *this );
    dialog->set_modal( true );
    dialog->set_current_folder( folder.c_str() );

    dialog->add_button("Cancel", RESPONSE_CANCEL);
    if( FILE_CHOOSER_ACTION_SAVE == fileAction )
        dialog->add_button("Save", RESPONSE_OK);
    if( FILE_CHOOSER_ACTION_OPEN == fileAction )
        dialog->add_button("Load", RESPONSE_OK);

    auto filter_bin = FileFilter::create();
    str.format( "%s files", extension );
    filter_bin->set_name( str.c_str() );
    str.format( "*.%s", extension );
    filter_bin->add_pattern( str.c_str() );
    dialog->add_filter( filter_bin );

    auto filter_any = FileFilter::create();
    filter_any->set_name( "Any files" );
    filter_any->add_pattern( "*" );
    dialog->add_filter( filter_any );

    int result = dialog->run();

    corestring filename;

    switch( result ) {
        case RESPONSE_OK: {
            filename = dialog->get_filename();
            break;
        }
        case RESPONSE_CANCEL: {
            break;
        }
        default: {
            break;
        }
    }

    if( FILE_CHOOSER_ACTION_SAVE == fileAction && extension != filename.substr( filename.find_last_of( "." ) + 1 )) {
        str.format( ".%s", extension );
        filename.append( str );
    }

    dialog->close();

    if( RESPONSE_OK == result ) {
        return filename;
    }
    return "";
}

///////////////////////////////////////
void MyWindow::on_image_load_clicked() {
///////////////////////////////////////
    if( !drawArea.gTypes.get_active_text().size() )
        return;

    int itemid = drawArea.itemTextToID( drawArea.gTypes.get_active_text().c_str() );

    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_OPEN, "png" );

    if( filename.size() ) {
        auto image = Gdk::Pixbuf::create_from_file( filename.c_str() );
        auto &destination = drawArea.binfile.items[ itemid ];
        destination.width = image->get_width();
        destination.height = image->get_height() / destination.imgCount;
        destination.count = destination.width * destination.height * destination.imgCount;
        destination.RGB32 = shared_ptr<unsigned int[]>( new unsigned int[ destination.count ]);
        if( image->get_has_alpha() ) {
            memcpy( destination.RGB32.get(), image->get_pixels(), destination.count * 4 );
        } else {
            unsigned char *rgbCols = ( unsigned char * ) image->get_pixels();
            int rowstride = image->get_rowstride();
            unsigned char *destCols = ( unsigned char * ) destination.RGB32.get();
            for( size_t y = 0; y < destination.height * destination.imgCount; ++y ) {
                for( size_t x = 0; x < destination.width; ++x ) {
                    RGBColor &rgbs = *( RGBColor * ) &rgbCols[ rowstride * y + x * sizeof( RGBColor )];
                    auto cols = &destCols[( destination.width * y + x ) * 4 ];
                    cols[ 0 ] = rgbs.r;
                    cols[ 1 ] = rgbs.g;
                    cols[ 2 ] = rgbs.b;
                    cols[ 3 ] = 0xff;
                }
            }
        }
        destination.toOrig();
        destination.toRGB32();
        drawArea.binfile.updateMaxHeight();
    }
};

///////////////////////////////////////
void MyWindow::on_image_save_clicked() {
///////////////////////////////////////
    if( !drawArea.gTypes.get_active_text().size() )
        return;

    int itemid = drawArea.itemTextToID( drawArea.gTypes.get_active_text().c_str() );

    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_SAVE, "png" );

    if( filename.size() ) {
        auto &item = drawArea.binfile.items[ itemid ];
        drawArea.view.getFromMemory(( unsigned char * ) item.RGB32.get(), item.width, item.height * item.imgCount );
        drawArea.view.img->save( filename, "png" );
    }
};

///////////////////////////////////////
void MyWindow::on_bin_file_load() {
///////////////////////////////////////
    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_OPEN, "bin" );

    if( filename.size() ) {
        readFile( filename );
    }
}

///////////////////////////////////////
void MyWindow::on_bin_file_save() {
///////////////////////////////////////
    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_SAVE, "bin" );

    if( filename.size() ) {
        saveFile( filename );
    }
}

///////////////////////////////////////
void MyWindow::on_bin_file_save_with_preview() {
///////////////////////////////////////
    drawArea.createPreview();
    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_SAVE, "bin" );

    if( filename.size() ) {
        saveFile( filename );
    }
}

///////////////////////////////////////
void MyWindow::readFile( ustring filename ) {
///////////////////////////////////////
    drawArea.setup( filename.c_str() );
    corestring title;
    title.format( "MK66 Watchface Editor - %s", filename.c_str() );
    set_title( title.c_str() );
}

///////////////////////////////////////
void MyWindow::saveFile( ustring filename ) {
///////////////////////////////////////
    drawArea.write( filename.c_str() );
}

///////////////////////////////////////
ExampleApplication::ExampleApplication( const char * appName ) : Application( appName ) {
///////////////////////////////////////
    set_application_name( appName );
    setlocale( LC_ALL, "C" );
}

///////////////////////////////////////
ExampleApplication::~ExampleApplication() {
///////////////////////////////////////
    on_window_hide();
}

///////////////////////////////////////
RefPtr<ExampleApplication> ExampleApplication::create( const char * appName ) {
///////////////////////////////////////
    return RefPtr<ExampleApplication>( new ExampleApplication( appName ));
}

///////////////////////////////////////
void ExampleApplication::on_startup() {
///////////////////////////////////////
    Application::on_startup();
}

///////////////////////////////////////
void ExampleApplication::on_activate() {
///////////////////////////////////////
    myWindow = RefPtr<MyWindow>( new MyWindow() );

    add_window( *myWindow.get() );

    myWindow->signal_hide().connect( sigc::mem_fun( *this, &ExampleApplication::on_window_hide ));
    add_action( "load", sigc::mem_fun( myWindow.get(), &MyWindow::on_bin_file_load ));
    add_action( "save", sigc::mem_fun( myWindow.get(), &MyWindow::on_bin_file_save ));
    add_action( "save_with_preview", sigc::mem_fun( myWindow.get(), &MyWindow::on_bin_file_save_with_preview ));
    add_action( "quit", sigc::mem_fun( *this, &ExampleApplication::on_menu_file_quit ));
    show_checkboard_action = add_action_bool( "show_checkboard", sigc::mem_fun( *this, &ExampleApplication::on_show_checkboard_clicked ));
    add_action( "about", sigc::mem_fun( *this, &ExampleApplication::on_menu_help_about ));

    auto app_menu = Gio::Menu::create();
    auto file = Gio::Menu::create();
    app_menu->append_submenu( "_File", file );
    file->append("_Load bin", "app.load");
    file->append("_Save bin", "app.save");
    file->append("Save bin with &preview", "app.save_with_preview" );
    file->append("_Quit", "app.quit");
    auto options = Gio::Menu::create();
    app_menu->append_submenu( "_Options", options );
    auto show_checkboard = Gio::MenuItem::create( "Sho_w checkboard", "app.show_checkboard" );
    options->append_item( show_checkboard );
    auto help = Gio::Menu::create();
    app_menu->append_submenu( "_Help", help );
    help->append("_About", "app.about");
    show_checkboard_action->set_enabled();

    set_menubar( app_menu );

    myWindow->show_all();
}

///////////////////////////////////////
void ExampleApplication::on_window_hide() {
///////////////////////////////////////
    auto window = myWindow.release(); ( void ) window;
}

///////////////////////////////////////
void ExampleApplication::on_menu_file_quit() {
///////////////////////////////////////
    vector<Window*> windows = get_windows();

    if ( windows.size() > 0 )
        windows[0]->hide();
}

///////////////////////////////////////
void ExampleApplication::on_show_checkboard_clicked() {
///////////////////////////////////////
    show_checkboard_action->get_state( myWindow->drawArea.showCheckboard );
    myWindow->drawArea.showCheckboard = !myWindow->drawArea.showCheckboard;
    auto newState = Glib::Variant<bool>::create( myWindow->drawArea.showCheckboard );
    show_checkboard_action->set_state( newState );
}

///////////////////////////////////////
void ExampleApplication::on_menu_help_about() {
///////////////////////////////////////
    MessageDialog messageBox( *myWindow.get(), "MK66 Watchface editor!\nCreated by János Klingl", true, MESSAGE_INFO, BUTTONS_OK, true );
    messageBox.set_title( "About" );
    messageBox.set_modal();
    messageBox.set_position( WindowPosition::WIN_POS_CENTER );
    messageBox.run();
}

///////////////////////////////////////
int main(int argc, char *argv[]) {
///////////////////////////////////////
    auto app = ExampleApplication::create( "mk66.watchface.editor.app" );

    return app->run( argc, argv );

    return 0;
}
