#include "visuals.h"

struct itemParams {
    int defImgCount;
    const char *name;
    bool formatted;
};

map<char, int> chrtopos= {{ '-', 0 },{ '.', 1 },{ '/', 2 },{ '0', 3 },{ '1', 4 },{ '2', 5 },{ '3', 6 },{ '4', 7 },{ '5', 8 },{ '6', 9 },{ '7', 10 },{ '8', 11 },{ '9', 12 },{ ':', 13 }};
map<int, corestring> defaults = {{ 1, "hour" },{ 2, "minute" },{ 3, "second" },{ 4, "hour" },{ 5, "minute" },{ 6, "second" },{ 11, "month" },{ 13, "day" },{ 15, "weekday" },{ 19, "80" },
                                { 21, "20" },{ 25, "2345" },{ 27, "year" },{ 30, "345" },{ 33, "100" },{ 47, "4320" }};
map<int, itemParams> itemparams= {{ 1, { 1, "hour-hand-image", false }}, { 2, { 1, "minute-hand-image", false }}, { 3, { 1, "second-hand-image", false }}, { 4, { 14, "hour-digit-text", true }},
    { 5, { 14, "minute-digit-text", true }}, { 6, { 14, "second-digit-text", true }}, { 8, { 5, "battery-indicator-image", false }}, { 9, { 2,"conntection-indicator-image", false }},
    { 10, { 1, "preview-image", false }}, { 11, { 14, "month-text", true }}, { 12, { 24, "month-names-image", false }}, { 13, { 14, "day-text", true }}, { 14, { 1, "month-day-separator", false }},
    { 15, { 14, "day-names-image", false }}, { 16, { 4, "AM-PM-image", false }}, { 17, { 1,"background-image", false }}, { 19, { 14, "bpm-text", true }}, { 20, { 16,"weather-icons", false }},
    { 21, { 14, "temperature-text", true }}, { 22, { 2, "temperature-image", false }}, { 23, { 2, "bpm-image", false }}, { 25, { 14, "steps-text", true }}, { 26, { 2, "steps-image", false }},
    { 27, { 14, "year-text", true }}, { 28, { 1, "year-month-separator", false }}, { 30, { 14, "kcal-text", true }}, { 31, { 2, "kcal-image", false }}, { 33, { 14, "battery-text", true }},
    { 34, { 1, "battery-icon", false }}, { 47, { 14, "distance-text", true }}, { 48, { 2, "distance-image", false }}, { 49, { 1, "hour-minute-separator", false }}
};

///////////////////////////////////////
Point::Point( double x, double y ) {
///////////////////////////////////////
    this->x = x;
    this->y = y;
}

///////////////////////////////////////
Point::Point( const Point &other ) {
///////////////////////////////////////
    x = other.x;
    y = other.y;
}

///////////////////////////////////////
Point &Point::operator = ( const Point &delta ) {
///////////////////////////////////////
    x = delta.x;
    y = delta.y;
    return *this;
}

///////////////////////////////////////
Point Point::operator - ( const Point &delta ) {
///////////////////////////////////////
    Point mp;
    mp.x = x - delta.x;
    mp.y = y - delta.y;
    return mp;
}

///////////////////////////////////////
Point Point::operator + ( const Point &delta ) {
///////////////////////////////////////
    Point mp;
    mp.x = x + delta.x;
    mp.y = y + delta.y;
    return mp;
}

///////////////////////////////////////
double Point::dist() const {
///////////////////////////////////////
    return sqrt( x * x + y * y );
}

///////////////////////////////////////
MyArea::MyArea() : posX( 0 ), posY( 0 ), watchfaceWidth( 240 ), watchfaceHeight( 280 ), shift( 0 ), preview( true ), debug( false ), button( 0 ), showCheckerboard( false ), filepos( 0 ) {
///////////////////////////////////////
    gPosXSpin.signal_changed().connect( sigc::mem_fun( this, &MyArea::on_item_posX_changed ));
    gPosYSpin.signal_changed().connect( sigc::mem_fun( this, &MyArea::on_item_posY_changed ));


    add_events( Gdk::KEY_PRESS_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK  | Gdk::BUTTON_RELEASE_MASK );
    signal_motion_notify_event().connect([&]( GdkEventMotion* event )->bool {
        if( button & 1 ) {
            Point pos;
            pos.x = event->x;
            pos.y = event->y;
            if( gTypes.get_active_text().size() || preview ) {
                if( gTypes.get_active_text().size() && ( event->state & GDK_SHIFT_MASK ) && preview ) {
                    auto delta = pressPosition - pos;
                    auto itemid = itemTextToID( gTypes.get_active_text().c_str() );
                    auto &item = binfile.items[ itemid ];
                    short x = item.posX - delta.x;
                    short y = item.posY - delta.y;
                    item.posX -= delta.x;
                    item.posY -= delta.y;
                    if( x < aPosX->get_lower() )
                        x = aPosX->get_lower();
                    if( x > aPosX->get_upper() )
                        x = aPosX->get_upper();
                    if( y < aPosY->get_lower() )
                        y = aPosY->get_lower();
                    if( y > aPosY->get_upper() )
                        y = aPosY->get_upper();
                    item.posX = x;
                    item.posY = y;
                    str.format( "%ld", item.posX );
                    gPosXSpin.set_text( str.c_str() );
                    gPosXSpin.set_value( item.posX );
                    str.format( "%ld", item.posY );
                    gPosYSpin.set_text( str.c_str() );
                    gPosYSpin.set_value( item.posY );
                    pressPosition = pos;
                }
                resetShift();
            } else {
                shift += pressPosition.y - pos.y;
                limitShift();
                pressPosition = pos;
            }
        }
        return false;
    }, false );
    signal_button_press_event().connect([&]( GdkEventButton* event )->bool {
        button |= event->button;
        pressPosition.x = event->x;
        pressPosition.y = event->y;
        grab_focus();
        return false;
    }, false );
    signal_button_release_event().connect([&]( GdkEventButton* event )->bool {
        button &= ~event->button;
        pressPosition.x = event->x;
        pressPosition.y = event->y;
        return false;
    }, false );
    signal_key_press_event().connect([&]( GdkEventKey* event )->bool {
        if( GDK_KEY_c == event->keyval ) {
            createPreview();
        }
        if( GDK_KEY_d == event->keyval ) {
            debug = !debug;
        }
        if( GDK_KEY_Up == event->keyval ) {
            gPosYSpin.spin( event->state & GDK_SHIFT_MASK ? Gtk::SPIN_PAGE_BACKWARD : Gtk::SPIN_STEP_BACKWARD, 1 );
            return true;
        }
        if( GDK_KEY_Down == event->keyval ) {
            gPosYSpin.spin( event->state & GDK_SHIFT_MASK ? Gtk::SPIN_PAGE_FORWARD : Gtk::SPIN_STEP_FORWARD, 1 );
            return true;
        }
        if( GDK_KEY_Left == event->keyval ) {
            gPosXSpin.spin( event->state & GDK_SHIFT_MASK ? Gtk::SPIN_PAGE_BACKWARD : Gtk::SPIN_STEP_BACKWARD, 1 );
            return true;
        }
        if( GDK_KEY_Right == event->keyval ) {
            gPosXSpin.spin( event->state & GDK_SHIFT_MASK ? Gtk::SPIN_PAGE_FORWARD : Gtk::SPIN_STEP_FORWARD, 1 );
            return true;
        }
        return false;
    }, false );

    signal_draw().connect( sigc::mem_fun( *this, &MyArea::on_draw ));
    doBackground();
    updateTypes();
    referenceTime = system_clock::now();
};

///////////////////////////////////////
MyArea::~MyArea() {};
///////////////////////////////////////

///////////////////////////////////////
void MyArea::updateTypes() {
///////////////////////////////////////
    gTypes.remove_all();

    for( auto &item : binfile.items ) {
        if( mapcontains( itemparams, item.second.type )) {
            str.format( "%ld - %s", item.first, itemparams[ item.second.type ].name );
            gTypes.append( str.c_str() );
        } else {
            str.format( "%ld - unknown", item.second.type );
            gTypes.append(  str.c_str() );
        }
    }

    gNewTypes.remove_all();

    for( auto &type : itemparams ) {
        if( binfile.hasitem( type.first ))
            continue;

        str.format( "%ld - %s", type.first, type.second.name );
        gNewTypes.append( str.c_str() );
    }
    initFields();
    binfile.updateMaxHeight();
}

///////////////////////////////////////
void MyArea::doBackground() {
///////////////////////////////////////
    int pixelcount = watchfaceWidth * watchfaceHeight;
    background.imgbuff.resize( pixelcount * 4 );
    unsigned int *pixels = ( unsigned int * ) background.imgbuff.data();
    memset( pixels, 0, pixelcount * 4 );
    int rounding = 54;
    const int halfx = watchfaceWidth / 2;
    const int halfy = watchfaceHeight / 2;
    for( int y = 0; y < watchfaceHeight; ++y ) {
        int dy = y - halfy;
        if( dy < 0 )
            ++dy;
        dy = fabs( dy );
        int ry = dy + rounding - halfy;
        for( int x = 0; x < watchfaceWidth; ++x ) {
            int dx = x - halfx;
            if( dx < 0 )
                ++dx;
            dx = fabs( dx );
            int rx = dx + rounding - halfx;
            bool draw = ( int ) sqrt( rx * rx + ry * ry ) > rounding - 1 && rx * dx > 0 && ry * dy > 0 ;
            pixels[ x + y * watchfaceWidth ] = draw ? 0x00000000 : 0xff000000;
        }
    }
    background.img = Gdk::Pixbuf::create_from_data(( const guint8 * ) pixels, Gdk::COLORSPACE_RGB, true, 8, watchfaceWidth, watchfaceHeight, watchfaceWidth * 4 );
}

///////////////////////////////////////
void MyArea::setup( const char * filename ) {
///////////////////////////////////////
    binfile.readFile( filename );
    signal_filename_changed.emit( filename );
    watchfaceWidth = binfile.hdr.w;
    watchfaceHeight = binfile.hdr.h;
    doBackground();
    updateTypes();
}

///////////////////////////////////////
void MyArea::setupDir( const char * directory ) {
///////////////////////////////////////
    DIR *dir = opendir( directory );
    struct dirent *dp;

    while (( dp = readdir( dir )) != nullptr ) {
        string filename = dp->d_name;

        if( filename.substr( filename.find_last_of( "." ) + 1 ) == "bin" )
            filenames.push_back( filename );
    }

    closedir( dir );
}

///////////////////////////////////////
void MyArea::write( const char * filename ) {
///////////////////////////////////////
    binfile.writeFile( filename );
}

///////////////////////////////////////
void MyArea::createPreview() {
///////////////////////////////////////
    if( !binfile.hasitem( 10 ) && binfile.hasitem( 17 ) ) {
        auto &background = binfile.items[ 17 ];
        imgitem item;
        item.type = 10;
        item.width = background.width;
        item.height = background.height;
        item.imgCount = itemparams[ item.type ].defImgCount;
        binfile.items.insert(pair<int, imgitem>( item.type, item ));
        updateTypes();
    }
    if( binfile.hasitem( 10 )) {
        auto &preview = binfile.items[ 10 ];
        preview.imgCount = 1;
        preview.count = preview.width * preview.height * preview.imgCount;
        Cairo::RefPtr<Cairo::ImageSurface> img = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, preview.width, preview.height );
        Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create( img );
        cr->save();
        cr->scale( 1.f * ( img->get_width() - 40 ) / img->get_width(), 1.f * ( img->get_height() - 39 ) / img->get_height() );
        cr->translate( 25, 23 );
        renderPreview( cr );
        cr->restore();
        int rounding = 80;
        const int halfx = img->get_width() / 2;
        const int halfy = img->get_height() / 2;
        int pixelcount = img->get_width() * img->get_height();
        preview.RGB32 = shared_ptr<unsigned int[]>( new unsigned int[ pixelcount ]);
        auto source = img->get_data();
        auto rgbcolor = preview.RGB32.get();
        for( int y = 0; y < img->get_height(); ++y ) {
            int dy = y - halfy;
            if( dy < 0 )
                ++dy;
            dy = fabs( dy );
            int ry = dy + rounding - halfy;
            for( int x = 0; x < img->get_width(); ++x ) {
                int dx = x - halfx;
                if( dx < 0 )
                    ++dx;
                dx = fabs( dx );
                int rx = dx + rounding - halfx;
                bool draw = false;
                draw |= dx <= halfx - rounding || dy <= halfy - rounding;
                draw |= ( int ) sqrt( rx * rx + ry * ry ) < rounding - 1 && rx * dx > 0 && ry * dy > 0;
                float round = 0;
                float dx2 = dx - halfx / 2;
                float del = halfy - halfx;
                float dy2 = dy - del - halfx / 2;
                float dist = sqrt( dx2 * dx2 + dy2 * dy2 );
                if( dy2 < 0 )
                    dist = dx2;
                if( dx2 < 0 )
                    dist = dy2;
                dist = pow( dist, 0.54 );
                round = 1 - dist / sqrt( rounding );
                if( dist > 7.35 ) {
                    if( dist < 7.8 ) {
                        dist = 7.1;
                        round = 1 - dist / sqrt( rounding );
                    }
                    unsigned int colorComp = 255 * round;
                    if( round < 0 )
                        colorComp = 0;
                    *rgbcolor = 255 << 24 | colorComp << 16 | colorComp << 8 | colorComp;
                } else {
                    *rgbcolor = 0xff << 24 | source[ 0 ] << 16 | source[ 1 ] << 8 | source[ 2 ];
                }
                source += 4;
                ++rgbcolor;
            }
        }
        preview.toOrig();
        preview.updateAlpha();
    }
}

///////////////////////////////////////
corestring MyArea::getDefault( int id ) {
///////////////////////////////////////
    time_t rawtime;
    struct tm *timeinfo;
    corestring def = "0";

    time( &rawtime );
    timeinfo = localtime( &rawtime );

    if( mapcontains( defaults, id )) {
        def = defaults[ id ];
        if( "hour" == def ) {
            def.format( "%ld", timeinfo->tm_hour );
        } else if( "minute" == def ) {
            def.format( "%ld", timeinfo->tm_min );
        } else if( "second" == def ) {
            def.format( "%ld", timeinfo->tm_sec );
        } else if( "year" == def ) {
            def.format( "%ld", timeinfo->tm_year + 1900 );
        } else if( "month" == def ) {
            def.format( "%ld", timeinfo->tm_mon + 1 );
        } else if( "day" == def ) {
            def.format( "%ld", timeinfo->tm_mday );
        } else if( "weekday" == def ) {
            def.format( "%ld", timeinfo->tm_wday );
        }
    }
    return def;
}

///////////////////////////////////////
int MyArea::itemTextToID( const char *name ) {
///////////////////////////////////////
    for( auto item : itemparams ) {
        if( 0 == strcmp( strstr( name, "- " ) + 2, item.second.name )) {
            return item.first;
        }
    }
    return 0;
}

///////////////////////////////////////
void MyArea::limitShift() {
///////////////////////////////////////
    if( shift > binfile.maxHeight - widgetHeight )
        shift = binfile.maxHeight - widgetHeight;

    if( shift < 0 )
        shift = 0;
}

///////////////////////////////////////
void MyArea::resetShift() {
///////////////////////////////////////
    shift = 0;
}

///////////////////////////////////////
void MyArea::initFields() {
///////////////////////////////////////
    resetShift();
    gTypes.set_active_text("");
    gNewTypes.set_active_text("");
    gDefValue.set_text("");
    gPosXSpin.set_text("");
    gPosYSpin.set_text("");
    grab_focus();
    gCopyImage.set_sensitive( false );
}

///////////////////////////////////////
void MyArea::renderPreview( const Cairo::RefPtr<Cairo::Context>& cr ) {
///////////////////////////////////////
    corestring output;

    if( binfile.hasitem( 17 ) && binfile.items[ 17 ].RGB32.get() ) {
        auto &background = binfile.items[ 17 ];
        view.getFromMemory(( unsigned char * )background.RGB32.get(), background.width, background.height );
        Gdk::Cairo::set_source_pixbuf( cr, view.img, background.posX, background.posY );
        cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
        cr->paint();
        cr->stroke();
    }

    for( auto &itempair : itemparams ) {
        const int id = itempair.first;
        auto &itemparam = itempair.second;
        if( 10 == id || 17 == id )
            continue;
        if( binfile.hasitem( id )) {
            auto &item = binfile.items[ id ];

            if( !item.RGB32.get() )
                continue;

            if( debug ) {
                output.format( "%02ld", id );
                cr->set_source_rgb( 1.0, 1.0, 1.0 );
                cr->set_font_size( 13 );
                cr->move_to( item.posX, item.posY + 13 );
                cr->show_text( output.c_str() );
                continue;
            }

            if( itemparam.formatted ) {
                int isize = 0;
                int xPos = 0;

                if ( 19 == id  )
                    isize = 3;
                else if ( 21 == id  )
                    isize = 4;
                else if ( 25 == id  )
                    isize = 5;
                else if ( 30 == id  )
                    isize = 4;
                else if ( 33 == id  )
                    isize = 3;
                else if ( 47 == id  )
                    isize = 5;

                auto def = getDefault( id );
                if ( 47 == id  )
                    output.format( "%1.2f", def.toLong() * 1e-3f );
                else if( mapcontains( defaults, id ))
                    output.format( "%02ld", def.toLong() );
                else
                    output.format( "%02ld", id );

                if( isize )
                    xPos = ( isize - ( int ) output.length() ) * item.width * 0.5f;

                for( auto chr : output ) {
                    view.getFromMemory(( unsigned char * )( item.RGB32.get() + chrtopos[ chr ] * item.width * item.height ), item.width, item.height );
                    Gdk::Cairo::set_source_pixbuf( cr, view.img, item.posX + xPos, item.posY );
                    cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                    cr->paint();
                    cr->stroke();
                    xPos += item.width;
                }
            } else {
                int pos = 0;

                if ( 1 <= id && id <= 3 )
                    continue;
                else if ( 8 == id  ) {
                    if( mapcontains( defaults, 33 )) {
                        int battery = defaults[ 33 ].toLong();
                        if( battery >= 0 && battery <= 100 )
                            pos = ( item.imgCount * ( defaults[ 33 ].toLong() + 5 )) / 100;
                        if( pos > ( item.imgCount - 1 ))
                            pos = ( item.imgCount - 1 );
                        if( pos < 0 )
                            pos = 0;
                    }
                } else if ( 12 == id  )
                    pos = 12;
                else if ( 15 == id  ) {
                    pos = getDefault( 15 ).toLong();
                    if( 7 <= pos || pos < 0 )
                        pos = 0;
                    pos += 7;
                } else if ( 20 == id  )
                    pos = 5;
                else if ( 23 == id || 26 == id || 31 == id )
                    pos = 1;
                else if ( 16 == id || 48 == id )
                    pos = ( item.imgCount == 2 ) ? 0 : 2;
                else
                    pos = 0;

                pos %= item.imgCount;
                view.getFromMemory(( unsigned char * )( item.RGB32.get() + ( pos * item.width * item.height )), item.width, item.height );
                cr->save();
                cr->translate( item.posX, item.posY );
                Gdk::Cairo::set_source_pixbuf( cr, view.img, 0, 0 );
                cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                cr->paint();
                cr->stroke();
                cr->restore();
            }
        }
    }
    for( int id = 1; id <=3; ++id ) {
        if( binfile.hasitem( id ) && binfile.items[ id ].RGB32.get() ) {
            corestring def;
            int hour = getDefault( 1 ).toLong();
            int minute = getDefault( 2 ).toLong();
            int second = getDefault( 3 ).toLong();
            auto &item = binfile.items[ id ];
            int shiftX = 0;
            int shiftY = 0;
            float rotateAngle = 0;
            /* calculate clock wise */
            int secs = hour * 3600l + minute * 60l + second * 1l;
            if( 1 == id )
                rotateAngle = 360 * ( secs / 3600.f / 12.f );
            else if ( 2 == id  )
                rotateAngle = 360 * ( secs / 3600.f );
            else if ( 3 == id  )
                rotateAngle = 360 * secs / 60.f;
            
            /* if clock hands have spin infomations */
            if( item.clockHandsInfo[1] || item.clockHandsInfo[2] || item.clockHandsInfo[3] ) { 
                /* spin center get from clockHandsInfo */
                int needleH = item.clockHandsInfo[1];
                int needleW = item.clockHandsInfo[3];
                int needleSpinAxis = item.clockHandsInfo[2];
                shiftX = -needleW / 2;      /* needle's spin X Axis */
                shiftY = -needleSpinAxis;   /* needle's spin Y Axis */
                view.getFromMemory(( unsigned char * ) item.RGB32.get(), item.width, item.height );
                cr->save();
                cr->translate( item.posX,  item.posY); /* set item position */
            }
            else {
                /* spin center become center of screen */
                shiftX = -item.width / 2;
                shiftY = - watchfaceHeight / 2 + ( item.posY ? ((item.posY - item.height) + (item.width / 2)) : 0 );
            view.getFromMemory(( unsigned char * ) item.RGB32.get(), item.width, item.height );
            cr->save();
            cr->translate( watchfaceWidth / 2, watchfaceHeight / 2 );
            }
            cr->rotate( rotateAngle * M_PI / 180 );
            cr->translate( shiftX, shiftY ); /* set center of spin */
            Gdk::Cairo::set_source_pixbuf( cr, view.img, 0, 0 );
            cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
            cr->paint();
            cr->stroke();
            cr->restore();
        }
    }
}

///////////////////////////////////////
bool MyArea::on_draw( const Cairo::RefPtr<Cairo::Context>& cr ) {
///////////////////////////////////////
    cr->save();

    Allocation allocation = get_allocation();
    widgetWidth = allocation.get_width();
    widgetHeight = allocation.get_height();

    Cairo::TextExtents te;

    limitShift();

    if( showCheckerboard ) {
        cr->set_line_width( 0 );
        int grid = 8;
        int xx = widgetWidth / grid;
        int xy = widgetHeight / grid + 2;
        int x = 0, y = 0;

        for( ;; ) {
            (( x + y ) & 1) ? cr->set_source_rgb( 0.3, 0.3, 0.3 ) : cr->set_source_rgb( 0.0, 0.1, 0.3 );
            cr->rectangle( x * grid, y * grid - shift % ( 2 * grid ), grid, grid );
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
    } else {
        cr->set_source_rgb( 0, 0, 0.0 );
        cr->rectangle( 0, 0, widgetWidth, widgetHeight );
        cr->fill();
        cr->stroke();
    }
    cr->set_font_size( 18 );
    cr->select_font_face( "Bitstream Vera Sans", Cairo::FontSlant::FONT_SLANT_NORMAL, Cairo::FontWeight::FONT_WEIGHT_BOLD );
    cr->set_source_rgb( 0.9, 0.9, 0.2 );

    corestring output;

    if( binfile.items.size() ) {
        if( preview ) {
            Cairo::RefPtr<Cairo::ImageSurface> previewImg = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, watchfaceWidth, watchfaceHeight );
            Cairo::RefPtr<Cairo::Context> cp = Cairo::Context::create( previewImg );
            cp->save();
            cp->set_source_rgb( 0, 0, 0.0 );
            cp->rectangle( 0, 0, watchfaceWidth , watchfaceHeight );
            cp->fill();
            cp->stroke();
            renderPreview( cp );
            cp->restore();

            unsigned char *source = previewImg->get_data();
            unsigned char *filter = background.img->get_pixels();
            for( int y = 0; y < previewImg->get_height(); ++y ) {
                for( int x = 0; x < previewImg->get_width(); ++x ) {
                    const int pos = 4 * ( x + y * previewImg->get_width() );
                    swap( source[ pos + 0 ], source[ pos + 2 ]);
                    if( filter[ pos + 3 ] == 0x00 )
                        source[ pos + 3 ] = 0x00;
                }
            }
            RefPtr< Gdk::Pixbuf > prvImage = Gdk::Pixbuf::create_from_data(( const guint8 * ) source, Gdk::COLORSPACE_RGB, true, 8, watchfaceWidth, watchfaceHeight, watchfaceWidth * 4 );
            Gdk::Cairo::set_source_pixbuf( cr, prvImage, 0, 0 );
            cr->rectangle( 0, 0, prvImage->get_width(), prvImage->get_height() );
            cr->paint();
            cr->stroke();

            if( gTypes.get_active_text().size() ) {
                int id = itemTextToID( gTypes.get_active_text().c_str() );
                if( binfile.hasitem( id )) {
                    float ms = duration_cast<milliseconds>( system_clock::now() - referenceTime ).count();
                    int isize = 1;
                    int xPos = 1;

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

                    auto def = getDefault( id );

                    if ( 47 == id  )
                        output.format( "%1.2f", def.toLong() * 1e-3f );
                    else if( mapcontains( defaults, id ))
                        output.format( "%02ld", def.toLong() );
                    else
                        output.format( "%02ld", id );

                    cr->save();
                    cr->set_source_rgba( 1.0, 1.0, 1.0, 1 - fabs( sin( ms * 0.005f )));
                    cr->set_line_width( 1 );
                    auto &item = binfile.items[ id ];
                    if( isize > 1 )
                        xPos = ( isize - ( int ) output.length() ) * item.width * 0.5f;
                    cr->rectangle( item.posX + xPos - 0.5f, item.posY + 0.5f, isize * item.width - 1, item.height - 1 );
                    cr->stroke();
                    cr->restore();
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

                if( 10 == item.first || 17 == item.first ) {
                    cr->save();
                    Gdk::Cairo::set_source_pixbuf( cr, background.img, x, y - shift );
                    cr->rectangle( 0, 0, background.img->get_width(), background.img->get_height() );
                    cr->paint();
                    cr->stroke();
                    cr->restore();
                }

                for( size_t i = 0; i < item.second.imgCount; ++i ) {
                    str.format( "%ld - %ld", item.first, item.second.imgCount );
                    cr->get_text_extents( str.c_str(), te );
                    cr->move_to( 0, y - shift + te.height );
                    cr->show_text( str.c_str() );

                    if( item.second.RGB32.get() ) {
                        cr->save();
                        view.getFromMemory(( unsigned char * ) item.second.RGB32.get() + pos, w, h );
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
            cr->set_source_rgb( 0.9, 0.9, 0.2 );
            if( gTypes.get_active_text().size() ) {
                int itemid = itemTextToID( gTypes.get_active_text().c_str() );
                auto &item = binfile.items[ itemid ];
                str.format( "%04d x %04d x %d", item.width, item.height, item.imgCount );
            } else {
                str.format( "%04d", shift );
            }
            cr->get_text_extents( str.c_str(), te );
            cr->move_to( widgetWidth - te.width - 5, widgetHeight - 5 );
            cr->show_text( str.c_str() );
        }
        if( has_focus() ) {
            cr->set_source_rgb( 0.0, 1.0, 0.0 );
        } else {
            cr->set_source_rgb( 0.4, 0.4, 0.4 );
        }
        cr->arc( widgetWidth - 18 , 16, 8.0, 0.0, 2.0 * M_PI);
        cr->fill_preserve();
    }


    cr->restore();
    return true;
}

///////////////////////////////////////
void MyArea::on_item_posX_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    auto &item = binfile.items[ itemid ];
    str = gPosXSpin.get_text().c_str();
    item.posX = str.toLong();
    if( item.posX > aPosX->get_upper() )
        item.posX = aPosX->get_upper();
};

///////////////////////////////////////
void MyArea::on_item_posY_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    auto &item = binfile.items[ itemid ];
    str = gPosYSpin.get_text().c_str();
    item.posY = str.toLong();
    if( item.posY > aPosY->get_upper() )
        item.posY = aPosY->get_upper();
};

///////////////////////////////////////
void MyArea::on_def_value_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    if( mapcontains( defaults, itemid )) {
        defaults[ itemid ] = gDefValue.get_text().c_str();
    }
}

///////////////////////////////////////
bool MyArea::on_window_key_pressed( GdkEventKey* event, Gtk::Widget *focus ) {
///////////////////////////////////////
    if( auto spin = dynamic_cast<SpinButton*>( focus )) {
        switch( event->keyval ) {
        case GDK_KEY_Page_Up:
            spin->spin( Gtk::SPIN_PAGE_FORWARD, 1 );
            return true;
        case GDK_KEY_Page_Down:
            spin->spin( Gtk::SPIN_PAGE_BACKWARD, 1 );
            return true;
        case GDK_KEY_KP_Add:
            spin->spin( event->state & GDK_SHIFT_MASK ? Gtk::SPIN_PAGE_FORWARD : Gtk::SPIN_STEP_FORWARD, 1 );
            return true;
        case GDK_KEY_KP_Subtract:
            spin->spin( event->state & GDK_SHIFT_MASK ? Gtk::SPIN_PAGE_BACKWARD : Gtk::SPIN_STEP_BACKWARD, 1 );
            return true;
        default:
            break;
        }
    }
    if( dynamic_cast<Entry*>( focus ) && focus != &gDefValue ) {
        if(( GDK_KEY_0 > event->keyval || GDK_KEY_9 < event->keyval ) && GDK_KEY_Left != event->keyval && GDK_KEY_Right != event->keyval &&
            GDK_KEY_BackSpace != event->keyval && GDK_KEY_Delete != event->keyval && GDK_KEY_End != event->keyval && GDK_KEY_Home != event->keyval &&
         ( focus != &gHeightFrame || GDK_KEY_minus != event->keyval )) {
            return true;
        }
    }
    if( filenames.size() ) {
        if( GDK_KEY_q == event->keyval ) {
            if( --filepos < 0 )
                filepos = filenames.size() - 1;

            if(( int ) filenames.size() > filepos ) {
                corestring file;
                file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str());
                setup( file.c_str() );
                resetShift();
            }
        }
        if( GDK_KEY_w == event->keyval ) {
            if( ++filepos >= ( int ) filenames.size() )
                filepos = 0;

            if(( int ) filenames.size() > filepos ) {
                corestring file;
                file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str() );
                setup( file.c_str() );
                resetShift();
            }
        }
    }
    return false;
}

///////////////////////////////////////
void MyArea::on_types_changed() {
///////////////////////////////////////
    gCopyImage.set_sensitive( true );
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    auto &item = binfile.items[ itemid ];
    aPosX = Gtk::Adjustment::create( item.posX, 0, watchfaceWidth - item.width, 1, item.width, 0 );
    gPosXSpin.set_adjustment( aPosX );
    aPosY = Gtk::Adjustment::create( item.posY, 0, watchfaceHeight - item.height, 1, item.height, 0 );
    gPosYSpin.set_adjustment( aPosY );
    str.format( "%ld", item.posX );
    gPosXSpin.set_text( str.c_str() );
    str.format( "%ld", item.posY );
    gPosYSpin.set_text( str.c_str() );
    if( mapcontains( defaults, itemid )) {
        gDefValue.set_text( defaults[ itemid ]);
    }
    gCopyImage.set_active( item.copyImage );
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
    item.imgCount = itemparams[ itemid ].defImgCount;
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
void MyArea::on_add_height_clicked() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    if( binfile.hasitem( itemid )) {
        auto &item = binfile.items[ itemid ];
        str = gHeightFrame.get_text().c_str();
        int plusHeight = str.toLong();

        int itemSize = item.width * item.height;
        int newItemSize = item.width * ( item.height + plusHeight );
        if( item.height + plusHeight < 0 )
            return;
        item.height += plusHeight;
        item.count = item.width * item.height * item.imgCount;

        shared_ptr<unsigned int[]> newRGB32 = shared_ptr<unsigned int[]>( new unsigned int[ item.count ]);
        for( int i = 0; i < item.imgCount; ++i ) {
            if( plusHeight < 0 ) {
                memcpy( &newRGB32[ i * newItemSize ], &item.RGB32[ i * itemSize ], newItemSize * sizeof( int ));
            } else {
                memcpy( &newRGB32[ i * newItemSize ], &item.RGB32[ i * itemSize ], itemSize * sizeof( int ));
                memset( &newRGB32[ i * newItemSize + itemSize ], 0, item.width * plusHeight * sizeof( int ));
            }
        }
        item.RGB32 = newRGB32;
    }
}

///////////////////////////////////////
void MyArea::on_copy_image_clicked() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() ) {
        gCopyImage.set_active( false );
        return;
    }

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    if( binfile.hasitem( itemid )) {
        auto &item = binfile.items[ itemid ];
        item.copyImage = gCopyImage.get_active();
        item.updateAlpha();
    }
}

///////////////////////////////////////
bool MyArea::on_timeout() {
///////////////////////////////////////
    queue_draw();
    return true;
}

///////////////////////////////////////
MyArea::type_filename_changed MyArea::filename_changed()
{
  return signal_filename_changed;
}
