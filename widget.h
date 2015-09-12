#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

const double r = 3;

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    void shift(const int &x, const int &y);
    void shift(const QPointF &p);
    void scale(const double &factor);

    void defaults();
    void reset();

private:
    void updatePaths();

protected:
    void timerEvent(QTimerEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void paintEvent(QPaintEvent *);

private:
    //Data
    QPainterPath curve;
    QPointF d11, d12, d21, d22;

    QVector<QPointF *> points, secPoints;
    double ec;

    //Service
    QPointF *moving, *hover;

    QPointF offset, lastPos;
    double scalef;

    //Options
    bool aa, showGrid, showAxes, showNumbers, showInfo, showHelp;
};

#endif //WIDGET_H
