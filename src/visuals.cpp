#include "visuals.h"

struct itemParams {
    int defimgcount;
    const char *name;
    bool formatted;
};

int watchface_width = 240;
int watchface_height = 280;

map<char, int> chrtopos= {{ '-', 0 },{ '.', 1 },{ '/', 2 },{ '0', 3 },{ '1', 4 },{ '2', 5 },{ '3', 6 },{ '4', 7 },{ '5', 8 },{ '6', 9 },{ '7', 10 },{ '8', 11 },{ '9', 12 },{ ':', 13 }};
map<int, corestring> defaults = {{ 1, "hour" },{ 2, "minute" },{ 3, "second" },{ 4, "hour" },{ 5, "minute" },{ 6, "second" },{ 11, "month" },{ 13, "day" },{ 19, "80" },{ 21, "20" },
                                { 25, "2345" },{ 27, "year" },{ 30, "345" },{ 33, "60" },{ 47, "4320" }};
map<int, itemParams> itemparams= {{ 1, { 1, "hour-hand-image", false }}, { 2, { 1, "minute-hand-image", false }}, { 3, { 1, "second-hand-image", false }}, { 4, { 14, "hour-digit-text", true }},
    { 5, { 14, "minute-digit-text", true }}, { 6, { 14, "second-digit-text", true }}, { 8, { 5, "battery-indicator-image", false }}, { 9, { 2,"conntection-indicator-image", false }},
    { 10, { 1, "preview-image", false }}, { 11, { 14, "month-text", true }}, { 12, { 24, "month-names-image", false }}, { 13, { 14, "day-text", true }}, { 14, { 1, "month-day-separator", false }},
    { 15, { 14, "day-names-image", false }}, { 16, { 4, "AM-PM-image", false }}, { 17, { 1,"background-image", false }}, { 19, { 14, "bpm-text", true }}, { 20, { 16,"weather-icons", false }},
    { 21, { 14, "temperature-text", true }}, { 22, { 2, "temperature-image", false }}, { 23, { 2, "bpm-image", false }}, { 25, { 14, "steps-text", true }}, { 26, { 2, "steps-image", false }},
    { 27, { 14, "year-text", true }}, { 28, { 1, "year-month-separator", false }}, { 30, { 14, "kcal-text", true }}, { 31, { 2, "kcal-image", false }}, { 33, { 14, "battery-text", true }},
    { 34, { 1, "battery-icon", false }}, { 47, { 14, "distance-text", true }}, { 48, { 2, "distance-image", false }}, { 49, { 1, "hour-minute-separator", false }}
};

///////////////////////////////////////
MyArea::MyArea() : posX( 0 ), posY( 0 ), buttonPressed( false ), shift( 0 ), preview( true ), debug( false ) {
///////////////////////////////////////
    int pixelcount = watchface_width * watchface_height;
    mask.imgbuff.resize( pixelcount * 4 );
    unsigned int *pixels = ( unsigned int * ) mask.imgbuff.data();
    memset( pixels, 0, pixelcount * 4 );
    int rounding = 54;
    const int halfx = watchface_width / 2;
    const int halfy = watchface_height / 2;
    for( int y = 0; y < watchface_height; ++y ) {
        int dy = y - halfy;
        if( dy < 0 )
            ++dy;
        dy = fabs( dy );
        int ry = dy + rounding - halfy;
        for( int x = 0; x < watchface_width; ++x ) {
            int dx = x - halfx;
            if( dx < 0 )
                ++dx;
            dx = fabs( dx );
            int rx = dx + rounding - halfx;
            bool draw = ( int ) sqrt( rx * rx + ry * ry ) > rounding - 1 && rx * dx > 0 && ry * dy > 0 ;
            pixels[ x + y * watchface_width ] = draw ? 0xff000000 : 0x00000000;
        }
    }
    mask.img = Gdk::Pixbuf::create_from_data(( const guint8 * ) pixels, Gdk::COLORSPACE_RGB, true, 8, watchface_width, watchface_height, watchface_width * 4 );

    signal_draw().connect( sigc::mem_fun( *this, &MyArea::on_draw ));
    str.format( "%ld", posX );
    gPosX.set_text( str.c_str() );
    str.format( "%ld", posY );
    gPosY.set_text( str.c_str() );
    str.format( "%ld", shift );
    gShift.set_text( str.c_str() );
    gDefvalue.set_text( "" );
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
    gPosX.set_text("0");
    gPosY.set_text("0");
    gDefvalue.set_text("");
}

///////////////////////////////////////
void MyArea::setup( const char * filename ) {
///////////////////////////////////////
    binfile.readFile( filename );
    watchface_width = binfile.hdr.w;
    watchface_height = binfile.hdr.h;
    updateTypes();
}

///////////////////////////////////////
void MyArea::write( const char * filename ) {
///////////////////////////////////////
    binfile.writeFile( filename );
}

///////////////////////////////////////
void MyArea::createPreview() {
///////////////////////////////////////
    if( binfile.hasitem( 10 )) {
        auto &background = binfile.items[ 10 ];
        Cairo::RefPtr<Cairo::ImageSurface> img = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, watchface_width, watchface_height );
        Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create( img );
        cr->set_source_rgb( 0, 0, 0 );
        cr->rectangle( 0, 0, img->get_width(), img->get_height() );
        cr->fill();
        cr->set_antialias( Cairo::ANTIALIAS_NONE );
        cr->set_source_rgb( 1, 1, 1 );
        cr->save();
        cr->scale( 1.f * ( img->get_width() - 40 ) / img->get_width(), 1.f * ( img->get_height() - 39 ) / img->get_height() );
        cr->translate( 25, 23 );
        renderPreview( cr );
        cr->restore();
        int rounding = 80;
        const int halfx = img->get_width() / 2;
        const int halfy = img->get_height() / 2;
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
                    cr->set_source_rgba( round, round, round, 1 );
                    cr->set_line_width( 1 );
                    cr->rectangle( x, y, 1.0, 1.0 );
                    cr->fill();
                }
            }
        }
        int pixelcount = img->get_width() * img->get_height();
        background.RGB32 = shared_ptr<unsigned int[]>( new unsigned int[ pixelcount ]);
        auto source = img->get_data();
        for( int y = 0; y < img->get_height(); ++y ) {
            for( int x = 0; x < img->get_width(); ++x ) {
                int pos = x + y * img->get_width();
                auto color = &source[ pos * 4 ];
                background.RGB32[ pos ] = 0xff << 24 | color[ 0 ] << 16 | color[ 1 ] << 8 | color[ 2 ];
            }
        }
        background.toOrig();
    }
}

///////////////////////////////////////
corestring MyArea::getDefault( int id ) {
///////////////////////////////////////
    time_t rawtime;
    struct tm *timeinfo;
    corestring def;

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
void MyArea::resetShift() {
///////////////////////////////////////
    shift = 0;
    str.format( "%ld", shift );
    gShift.set_text( str.c_str() );
}

///////////////////////////////////////
void MyArea::renderPreview( const Cairo::RefPtr<Cairo::Context>& cr ) {
///////////////////////////////////////
    corestring clock;
    corestring output;

    if( binfile.hasitem( 17 )) {
        auto &background = binfile.items[ 17 ];
        view.getFromMemory(( unsigned char * )background.RGB32.get(), background.width, background.height );
        Gdk::Cairo::set_source_pixbuf( cr, view.img, background.posx, background.posy );
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
                cr->move_to( item.posx, item.posy + 13 );
                cr->show_text( output.c_str() );
                continue;
            }

            if( itemparam.formatted ) {
                int isize = 0;
                int xpos = 0;

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
                if( 4 == id )
                    output.format( "%02ld", def.toLong() );
                else if ( 5 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 6 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 11 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 13 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 19 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 21 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 25 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 27 == id  )
                    output.format( "%04ld", def.toLong() );
                else if ( 30 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 33 == id  )
                    output.format( "%02ld", def.toLong() );
                else if ( 47 == id  )
                    output.format( "%1.2f", def.toLong() * 1e-3f );
                else
                    output.format( "%02ld", id );

                if( isize )
                    xpos = ( isize - ( int ) output.length() ) * item.width * 0.5f;

                for( auto chr : output ) {
                    view.getFromMemory(( unsigned char * )( item.RGB32.get() + chrtopos[ chr ] * item.width * item.height ), item.width, item.height );
                    Gdk::Cairo::set_source_pixbuf( cr, view.img, item.posx + xpos, item.posy );
                    cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                    cr->paint();
                    cr->stroke();
                    xpos += item.width;
                }
            } else {
                int pos = 0;

                if ( 1 <= id && id <= 3 )
                    continue;
                else if ( 8 == id  ) {
                    if( mapcontains( defaults, 33 )) {
                        int battery = defaults[ 33 ].toLong();
                        if( battery >= 0 && battery <= 100 )
                            pos = ( item.imgcount * ( defaults[ 33 ].toLong() + 10 )) / 100;
                        if( pos > ( item.imgcount - 1 ))
                            pos = ( item.imgcount - 1 );
                        if( pos < 0 )
                            pos = 0;
                    }
                } else if ( 12 == id  )
                    pos = 12;
                else if ( 15 == id  )
                    pos = 7;
                else if ( 20 == id  )
                    pos = 5;
                else if ( 23 == id || 26 == id || 31 == id )
                    pos = 1;
                else if ( 16 == id || 48 == id )
                    pos = item.imgcount == 2 ? 0 : 2;
                else
                    pos = 0;

                view.getFromMemory(( unsigned char * )( item.RGB32.get() + ( pos * item.width * item.height )), item.width, item.height );
                cr->save();
                cr->translate( item.posx, item.posy );
                Gdk::Cairo::set_source_pixbuf( cr, view.img, 0, 0 );
                cr->rectangle( 0, 0, view.img->get_width(), view.img->get_height() );
                cr->paint();
                cr->stroke();
                cr->restore();
            }
        }
    }
    for( int id = 1; id <=3; ++id ) {
        if( binfile.hasitem( id )) {
            corestring def;
            int hour = getDefault( 1 ).toLong();
            int minute = getDefault( 2 ).toLong();
            int second = getDefault( 3 ).toLong();
            auto &item = binfile.items[ id ];
            int shiftx = 0;
            int shifty = 0;
            float rotateAngle = 0;
            shiftx = -item.width / 2;
            shifty = - watchface_height / 2 + ( item.posy ? item.posy - item.height + item.width / 2 : 0 );
            int secs = hour * 3600l + minute * 60l + second * 1l;
            if( 1 == id )
                rotateAngle = 360 * ( secs / 3600.f / 12.f );
            else if ( 2 == id  )
                rotateAngle = 360 * ( secs / 3600.f );
            else if ( 3 == id  )
                rotateAngle = 360 * secs / 60.f;
            view.getFromMemory(( unsigned char * ) item.RGB32.get(), item.width, item.height );
            cr->save();
            cr->translate( watchface_width / 2, watchface_height / 2 );
            cr->rotate( rotateAngle * M_PI / 180 );
            cr->translate( shiftx, shifty );
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

    int font_size = 18;
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

    corestring output;

    if( binfile.items.size() ) {
        if( preview ) {
            renderPreview( cr );
            if( gTypes.get_active_text().size() ) {
                int id = itemTextToID( gTypes.get_active_text().c_str() );
                if( binfile.hasitem( id )) {
                    float ms = duration_cast<milliseconds>( system_clock::now() - referenceTime ).count();
                    int isize = 1;
                    int xpos = 1;

                    corestring def = getDefault( id );
                    if( 4 == id )
                        output.format( "%02ld", def.toLong() );
                    else if ( 5 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 6 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 11 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 13 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 19 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 21 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 25 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 27 == id  )
                        output.format( "%04ld", def.toLong() );
                    else if ( 30 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 33 == id  )
                        output.format( "%02ld", def.toLong() );
                    else if ( 47 == id  )
                        output.format( "%1.2f", def.toLong() * 1e-3f );
                    else
                        output.format( "%02ld", id );

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
                    cr->save();
                    cr->set_source_rgba( 1.0, 1.0, 1.0, 1 - fabs( sin( ms * 0.005f )));
                    cr->set_line_width( 1 );
                    auto &item = binfile.items[ id ];
                    if( isize > 1 )
                        xpos = ( isize - ( int ) output.length() ) * item.width * 0.5f;
                    cr->rectangle( item.posx + xpos - 0.5f, item.posy + 0.5f, isize * item.width - 1, item.height - 1 );
                    cr->stroke();
                    cr->restore();
                }
            }

            Gdk::Cairo::set_source_pixbuf( cr, mask.img, 0, 0 );
            cr->rectangle( 0, 0, mask.img->get_width(), mask.img->get_height() );
            cr->paint();
            cr->stroke();
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
        }
    }

    str.format( "%04x %04x %04x", posX, posY, shift );
    cr->get_text_extents( str.c_str(), te );
    cr->move_to( widgetWidth - te.width - 5, widgetHeight - 5 );
    cr->show_text( str.c_str() );

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
void MyArea::on_def_value_changed() {
///////////////////////////////////////
    if( !gTypes.get_active_text().size() )
        return;

    int itemid = itemTextToID( gTypes.get_active_text().c_str() );
    if( mapcontains( defaults, itemid )) {
        defaults[ itemid ] = gDefvalue.get_text().c_str();
    }
}

///////////////////////////////////////
void MyArea::on_mouse_moved( mousePosition pos ) {
///////////////////////////////////////
    if( buttonPressed ) {
        if( gTypes.get_active_text().size() || preview ) {
            resetShift();
        } else {
            shift += mousePressPosition.y - pos.y;

            if( shift > binfile.maxHeight - widgetHeight )
                shift = binfile.maxHeight - widgetHeight;

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
    if( mapcontains( defaults, itemid )) {
        gDefvalue.set_text( defaults[ itemid ]);
    }
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
        item.count = item.width * item.height * item.imgcount;

        shared_ptr<unsigned int[]> newRGB32 = shared_ptr<unsigned int[]>( new unsigned int[ item.count ]);
        for( int i = 0; i < item.imgcount; ++i ) {
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
bool MyArea::on_timeout() {
///////////////////////////////////////
    queue_draw();
    return true;
}
