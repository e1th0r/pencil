#include "polylinetool.h"

#include "editor.h"
#include "scribblearea.h"

#include "strokemanager.h"
#include "layermanager.h"

#include "layervector.h"
#include "layerbitmap.h"


PolylineTool::PolylineTool( QObject *parent ) :
BaseTool( parent )
{
}

ToolType PolylineTool::type()
{
    return POLYLINE;
}

void PolylineTool::loadSettings()
{
    QSettings settings( "Pencil", "Pencil" );

    properties.width = settings.value( "polyLineWidth" ).toDouble();
    properties.feather = -1;
    properties.pressure = ON;
    properties.invisibility = OFF;
    properties.preserveAlpha = OFF;

    if ( properties.width <= 0 )
    {
        properties.width = 1.5;
        settings.setValue( "polyLineWidth", properties.width );
    }
}

QCursor PolylineTool::cursor() //Not working this one, any guru to fix it?
{
    if ( isAdjusting ) { // being dynamically resized
        return QCursor( this->circleCursors() ); // two circles cursor
        qDebug() << "adjusting";
    }
    return Qt::CrossCursor;
}

void PolylineTool::clear()
{
    points.clear();
}

void PolylineTool::mousePressEvent( QMouseEvent *event )
{
    Layer* layer = mEditor->layers()->currentLayer();

    if ( event->button() == Qt::LeftButton )
    {
        if ( layer->type() == Layer::BITMAP || layer->type() == Layer::VECTOR )
        {
            if ( points.size() == 0 )
            {
                mEditor->backup( tr( "Line" ) );
            }

            if ( layer->type() == Layer::VECTOR )
            {
                ( ( LayerVector * )layer )->getLastVectorImageAtFrame( mEditor->currentFrame(), 0 )->deselectAll();
                if ( mScribbleArea->makeInvisible() && !mScribbleArea->showThinLines() )
                {
                    mScribbleArea->toggleThinLines();
                }
            }
            points << getCurrentPoint();
            mScribbleArea->setAllDirty();
        }
    }
}

void PolylineTool::mouseReleaseEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
}

void PolylineTool::mouseMoveEvent( QMouseEvent *event )
{
    Q_UNUSED( event );
    Layer* layer = mEditor->layers()->currentLayer();

    if ( layer->type() == Layer::BITMAP || layer->type() == Layer::VECTOR )
    {
        mScribbleArea->drawPolyline( points, getCurrentPoint() );
    }
}

void PolylineTool::mouseDoubleClickEvent( QMouseEvent *event )
{
    // XXX highres position ??
    if ( BezierCurve::eLength( m_pStrokeManager->getLastPressPixel() - event->pos() ) < 2.0 )
    {
        mScribbleArea->endPolyline( points );
        clear();
    }
}

bool PolylineTool::keyPressEvent( QKeyEvent *event )
{
    switch ( event->key() ) {
    case Qt::Key_Return:
        if ( points.size() > 0 )
        {
            mScribbleArea->endPolyline( points );
            clear();
            return true;
        }
        break;

    case Qt::Key_Escape:
        if ( points.size() > 0 ) {
            clear();
            return true;
        }
        break;

    default:
        return false;
    }

    return false;
}