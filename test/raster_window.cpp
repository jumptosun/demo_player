#include "raster_window.h"

//! [1]
RasterWindow::RasterWindow(QWindow *parent)
    : QWindow(parent)
    , m_backingStore(new QBackingStore(this))
{
    setGeometry(100, 100, 300, 200);
}
//! [1]

RasterWindow::~RasterWindow()
{
    delete m_backingStore;
}


//! [7]
bool RasterWindow::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        renderNow();
        return true;
    }
    return QWindow::event(event);
}
//! [7]

//! [6]
void RasterWindow::renderLater()
{
    requestUpdate();
}
//! [6]


//! [5]
void RasterWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    m_backingStore->resize(resizeEvent->size());
}
//! [5]

//! [2]
void RasterWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed())
        renderNow();
}
//! [2]


//! [3]
void RasterWindow::renderNow()
{
    if (!isExposed())
        return;

    QRect rect(0, 0, width(), height());
    m_backingStore->beginPaint(rect);

    QPaintDevice *device = m_backingStore->paintDevice();
    QPainter painter(device);

    painter.fillRect(0, 0, width(), height(), QGradient::NightFade);
    render(&painter);
    painter.end();

    m_backingStore->endPaint();
    m_backingStore->flush(rect);
}
//! [3]

//! [4]
void RasterWindow::render(QPainter *painter)
{
    QCoreApplication* app = QGuiApplication::instance();

    // TODO: get filepath from first command arguement
    QStringList args = app->arguments();

    // TODO: load QImage from file
    // 至少需要1个参数（程序自身路径+文件路径）
    if(args.size() > 1) {
        QString filePath = args.at(1); // 第一个用户参数是索引1
        QImage image(filePath);
        QPixmap pixmap = QPixmap::fromImage(image);

        if(!image.isNull()) {
            // 绘制图片到窗口中央
            QRectF targetRect(0, 0, width(), height());
            painter->drawPixmap(targetRect, pixmap, pixmap.rect());
            return;
        }
    }
}
//! [4]