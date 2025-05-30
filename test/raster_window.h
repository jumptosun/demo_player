#include <QtGui>

class RasterWindow : public QWindow
{
    Q_OBJECT
public:
    explicit RasterWindow(QWindow *parent = 0);
    virtual ~RasterWindow();

    virtual void render(QPainter *painter);

public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

private:
    QBackingStore *m_backingStore;
};