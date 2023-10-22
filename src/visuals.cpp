#include "visuals.h"

struct itemParams {
    int defimgcount;
    const char *name;
};

map<char, int> chrtopos= {{'-', 0},{'.', 1},{'/', 2},{'0', 3},{'1', 4},{'2', 5},{'3', 6},{'4', 7},{'5', 8},{'6', 9},{'7', 10},{'8', 11},{'9', 12},{':', 13}};
vector<int> format = { 4, 5, 6, 11, 13, 19, 21, 25, 27, 30, 33, 47 };
vector<int> imgs = { 8, 9, 12, 14, 15, 16, 18, 20, 22, 23, 24, 26, 28, 29, 31, 32, 34, 48, 49 };
map<int, itemParams> itemparams= {{1, {1,"hour-hand-image"}},{2, {1,"minute-hand-image"}},{3, {1,"second-hand-image"}},{4, {14,"hour-digit-text"}},{5, {14,"minute-digit-text"}},
    {6, {14,"second-digit-text"}},{8, {5,"battery-icons"}},{9, {2,"conntection-indicator-image"}},{10, {1,"preview-image"}},{11, {14,"month-text"}},{12, {24,"month-names-image"}},
    {13, {14,"day-text"}},{14, {14,"month-day-separator-dash"}},{15, {14,"day-names-image"}},{16, {4,"AM-PM"}},{17, {1,"background-image"}},
    {19, {14,"bpm-text"}},{20, {16,"weather-icons"}},{21, {14,"temperature-text"}},{22, {2,"temperature-image"}},
    {23, {2,"bpm-image"}},{25, {14,"steps-count-text"}},{26, {2,"steps-image"}},{27, {14,"year-text"}},{28, {14,"year-month-separator-dash"}},
    {30, {14,"kcal-text"}},{31, {2,"kcal-image"}},{33, {14,"battery-text"}},{34, {1,"battery-icon"}},{47, {14,"distance-text"}},{48, {2,"distance-image"}},{49, {1,"hour-minute-separator-image"}}
};

///////////////////////////////////////
MyArea::MyArea() : posX( 0 ), posY( 0 ), buttonPressed(false), shift( 0 ), preview( true ), debug( false ) {
///////////////////////////////////////
    signal_draw().connect( sigc::mem_fun( *this, &MyArea::on_draw ));
    str.format( "%ld", posX );
    gPosX.set_text( str.c_str() );
    str.format( "%ld", posY );
    gPosY.set_text( str.c_str() );
    str.format( "%ld", shift );
    gShift.set_text( str.c_str() );
};

///////////////////////////////////////
MyArea::~MyArea() {};
///////////////////////////////////////

///////////////////////////////////////
void MyArea::updateTypes() {
///////////////////////////////////////
    gTypes.remove_all();

    for( auto &item : binfile.items ) {
        if( itemparams.find( item.second.type ) != itemparams.end() )
            gTypes.append( itemparams[ item.second.type ].name );
        else {
            str.format( "%ld - unknown", item.second.type );
            gTypes.append(  str.c_str() );
        }
    }

    gNewTypes.remove_all();

    for( auto &type : itemparams ) {
        if( binfile.items.find( type.first ) != binfile.items.end() )
            continue;

        gNewTypes.append( type.second.name );
    }
}

///////////////////////////////////////
void MyArea::setup( const char * filename ) {
///////////////////////////////////////
    binfile.readFile( filename );
    updateTypes();
}

///////////////////////////////////////
void MyArea::write( const char * filename ) {
///////////////////////////////////////
    binfile.writeFile( filename );
}

///////////////////////////////////////
void MyArea::saveFile() {
///////////////////////////////////////
    corestring savefile;
    savefile.format( "%s.png", binfile.curfilename.c_str() );
    auto pixels = Gdk::Pixbuf::create(this->get_window(), 0, 0, 240, 280);
    savefile.format( "%s.png", binfile.curfilename.c_str() );
    pixels->save( savefile.c_str(), "png" );

    if( binfile.items.find( 10 ) != binfile.items.end() ) {
        auto &preview = binfile.items[ 10 ];

        savefile.format( "%s.10.png", binfile.curfilename.c_str() );
        RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create_from_data((const guint8*) preview.rgb32.get(), Gdk::COLORSPACE_RGB, true, 8, preview.width, preview.height, preview.width * 4 );
        image->save( savefile.c_str(), "png" );
        image.reset();
    }
}

///////////////////////////////////////
int MyArea::itemTextToID( const char *name ) {
///////////////////////////////////////
    for( auto item : itemparams ) {
        if( 0 == strcmp( name, item.second.name )) {
            return item.first;
        }
    }
    return 0;
}

///////////////////////////////////////
void MyArea::resetShift() {
///////////////////////////////////////
    shift = 0;
    str.format( "%ld", shift );
    gShift.set_text( str.c_str() );
}

///////////////////////////////////////
bool MyArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
///////////////////////////////////////

    cr->save();

    Allocation allocation = get_allocation();
    widget_width = allocation.get_width();
    widget_height = allocation.get_height();

    Cairo::TextExtents te;

    int font_size = widget_height / 42;
    int text_height = font_size * 1.15;
    int text_ypos = text_height * 1.5;

    text_ypos = text_height * 1.4;
#if 0 // show checkboard;
    cr->set_line_width( 0 );
    int grid = 8;
    int xx = widget_width / grid;
    int xy = widget_height / grid;
    int x = 0, y = 0;

    for( ;; ) {
        (( x + y ) & 1) ? cr->set_source_rgb( 0.2, 0.2, 0.2 ) : cr->set_source_rgb( 0, 0, 0.0 );
        cr->rectangle( x * grid, y * grid, grid, grid );
        cr->fill();
        cr->stroke();
        ++x;

        if( x > xx ) {
            x = 0;
            ++y;

            if( y > xy )
                break;
        }
    }

#endif
    cr->set_font_size( font_size );
    cr->select_font_face( "Bitstream Vera Sans",Cairo::FontSlant::FONT_SLANT_NORMAL, Cairo::FontWeight::FONT_WEIGHT_BOLD );
    cr->set_source_rgb( 0.9, 0.9, 0.2 );

    if( binfile.items.size() ) {
        if( preview ) {
            time_t rawtime;
            struct tm *timeinfo;
            corestring clock;

            text_ypos = text_height * 1.4;
            time( &rawtime );

            timeinfo = localtime( &rawtime );

            corestring output;
            int xpos = 0;
            if( binfile.hasitem( 17 )) {
                auto &background = binfile.items[ 17 ];
                view.getFromMemory(( unsigned char * )background.rgb32.get(), background.width, background.height );
                Gdk::Cairo::set_source_pixbuf( cr, view.img, background.posx, background.posy );
                cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                cr->paint();
                cr->stroke();
            }

            for( auto &id : imgs ) {
                if( binfile.hasitem( id )) {
                    auto &clock = binfile.items[ id ];

                    if( !clock.rgb32.get() )
                        continue;

                    int pos = 0;

                    if ( 8 == id  )
                        pos = 4;
                    else if ( 12 == id  )
                        pos = timeinfo->tm_mon + 12;
                    else if ( 15 == id  )
                        pos = timeinfo->tm_wday + 7;
                    else if ( 20 == id  )
                        pos = 5;
                    else if ( 23 == id || 26 == id || 31 == id )
                        pos = 1;
                    else if ( 16 == id || 48 == id )
                        pos = clock.imgcount == 2 ? 0 : 2;
                    else
                        pos = 0;

                    xpos = 0;
                    view.getFromMemory(( unsigned char * )( clock.rgb32.get() + ( pos * clock.width * clock.height )), clock.width, clock.height );
                    Gdk::Cairo::set_source_pixbuf( cr, view.img, clock.posx + xpos, clock.posy );
                    cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                    cr->paint();
                    cr->stroke();
                }
            }

            for( auto &id : format ) {
                if( binfile.hasitem( id )) {
                    auto &clock = binfile.items[ id ];

                    if( !clock.rgb32.get() )
                        continue;

                    int isize = 0;
                    xpos = 0;

                    if ( 19 == id  )
                        isize = 3;
                    else if ( 21 == id  )
                        isize = 3;
                    else if ( 25 == id  )
                        isize = 5;
                    else if ( 30 == id  )
                        isize = 4;
                    else if ( 33 == id  )
                        isize = 3;
                    else if ( 47 == id  )
                        isize = 5;

                    if( debug ) {
                        output.format( "%02ld", id );
                    } else {
                        if( 4 == id )
                            output.format( "%02ld", timeinfo->tm_hour );
                        else if ( 5 == id  )
                            output.format( "%02ld", timeinfo->tm_min );
                        else if ( 6 == id  )
                            output.format( "%02ld", timeinfo->tm_sec );
                        else if ( 11 == id  )
                            output.format( "%02ld", timeinfo->tm_mon + 1 );
                        else if ( 13 == id  )
                            output.format( "%02ld", timeinfo->tm_mday );
                        else if ( 19 == id  )
                            output.format( "%02ld", 80 );
                        else if ( 21 == id  )
                            output.format( "%02ld", 20 );
                        else if ( 25 == id  )
                            output.format( "%02ld", 2345 );
                        else if ( 27 == id  )
                            output.format( "%04ld", timeinfo->tm_year + 1900 );
                        else if ( 30 == id  )
                            output.format( "%02ld", 345 );
                        else if ( 33 == id  )
                            output.format( "%02ld", 60 );
                        else if ( 47 == id  )
                            output.format( "%1.2f", 4.32f );
                        else
                            output.format( "%02ld", id );
                    }

                    if( isize )
                        xpos = ( isize - ( int ) output.length() ) * clock.width * 0.5f;

                    for( auto chr : output) {
                        view.getFromMemory(( unsigned char * )( clock.rgb32.get() + chrtopos[ chr ] * clock.width * clock.height ), clock.width, clock.height );
                        Gdk::Cairo::set_source_pixbuf( cr, view.img, clock.posx + xpos, clock.posy );
                        cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                        cr->paint();
                        cr->stroke();
                        xpos += clock.width;
                    }
                }
            }
        } else {
            size_t pos = 0;
            size_t y = 0;
            string text = gTypes.get_active_text();
            int id = 0;

            if( text.size() ) {
                id = itemTextToID( text.c_str() );
            }

            for( auto item : binfile.items ) {
                if( id && item.second.type != id )
                    continue;

                int w = item.second.width;
                int h = item.second.height;

                size_t x = 80;
                pos = 0;

                for( size_t i = 0; i < item.second.imgcount; ++i ) {
                    str.format( "%ld - %ld", item.first, item.second.imgcount );
                    cr->get_text_extents( str.c_str(), te );
                    cr->move_to( 0, y - shift + te.height );
                    cr->show_text( str.c_str() );

                    if( item.second.rgb32.get() ) {
                        cr->save();
                        view.getFromMemory(( unsigned char * ) item.second.rgb32.get() + pos, w, h );
                        Gdk::Cairo::set_source_pixbuf( cr, view.img, x, y - shift );
                        cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                        cr->paint();
                        cr->stroke();
                        cr->restore();
                    }

                    x += w;
                    pos += w * h * 4;
                }

                y += h;
            }
        }
    }

    str.format( "%04x %04x %04x", posX, posY, shift );
    cr->get_text_extents( str.c_str(), te );
    cr->move_to( widget_width - te.width - 5, widget_height - 5 );
    text_ypos += text_height;
    cr->show_text( str.c_str() );

    text_ypos = text_height * 1.4;

    cr->restore();
    return true;
}

///////////////////////////////////////
void MyArea::on_width_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    auto &item = binfile.items[ itemid ];
    str = gPosX.get_text().c_str();
    item.posx = str.toLong();
};

///////////////////////////////////////
void MyArea::on_height_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    auto &item = binfile.items[ itemid ];
    str = gPosY.get_text().c_str();
    item.posy = str.toLong();
};

///////////////////////////////////////
void MyArea::on_mouse_moved( mousePosition pos ) {
///////////////////////////////////////
    if( buttonPressed ) {
        if( gTypes.get_active_text().size() || preview ) {
            resetShift();
        } else {
            shift += mousePressPosition.y - pos.y;

            if( shift > binfile.maxheight - widget_height )
                shift = binfile.maxheight - widget_height;

            if( shift < 0 )
                shift = 0;

            str.format( "%ld", shift );
            gShift.set_text( str.c_str() );
            mousePressPosition = pos;
        }
    }
};

///////////////////////////////////////
void MyArea::on_mouse_pressed( uint button, mousePosition pos ) {
///////////////////////////////////////
    if( button & 1 ) {
        mousePressPosition = pos;
        buttonPressed = true;
    }
};

///////////////////////////////////////
void MyArea::on_mouse_released( uint button, mousePosition pos ) {
///////////////////////////////////////
    if( button & 1 ) {
        buttonPressed = false;
    }
};

///////////////////////////////////////
void MyArea::on_types_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    auto &item = binfile.items[ itemid ];
    str.format( "%ld", item.posx );
    gPosX.set_text( str.c_str() );
    str.format( "%ld", item.posy );
    gPosY.set_text( str.c_str() );
    resetShift();
}

///////////////////////////////////////
void MyArea::on_add_clicked() {
///////////////////////////////////////
    if( !gNewTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gNewTypes.get_active_text().c_str() );
    imgitem item;
    item.type = itemid;
    item.width = 16;
    item.height = 16;
    item.imgcount = itemparams[ itemid ].defimgcount;
    binfile.items.insert(pair<int, imgitem>( itemid, item ));
    updateTypes();
}

///////////////////////////////////////
void MyArea::on_del_clicked() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    binfile.items.erase( itemid );
    updateTypes();
}

///////////////////////////////////////
bool MyArea::on_timeout() {
///////////////////////////////////////
    queue_draw();
    return true;
}
