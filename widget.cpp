#include "widget.h"
#include <QtGui>
#include <QDesktopWidget>

Widget::Widget(QWidget *parent)
    : QWidget(parent) {
    QDesktopWidget dw;
    resize(dw.size() * 3 / 4);
    move(dw.rect().center() - rect().center());
    setMinimumSize(dw.size() / 5);

    setWindowTitle("Eccentricity v2.3");
    setAttribute(Qt::WA_MouseTracking);

    defaults();

    startTimer(1);
}

Widget::~Widget() {
}

void Widget::shift(const int &x, const int &y) {
    offset += QPointF(x, y);
}
void Widget::shift(const QPointF &p) {
    offset += p;
}
void Widget::scale(const double &factor) {
    scalef *= factor;
}

void Widget::defaults() {
    aa = true;
    showInfo = true;
    showHelp = false;
    showGrid = true;
    showAxes = true;
    showNumbers = true;

    reset();
}

void Widget::reset() {
    offset = QPointF();
    lastPos = QPointF();
    scalef = 50;

    points.clear();
    points << new QPointF(0, 0);
    points << new QPointF(-3, 0);
    points << new QPointF(0, 3);

    moving = 0;
    hover = 0;
    ec = 1;

    secPoints.clear();
    secPoints << new QPointF;
    secPoints << new QPointF;

    updatePaths();
}

void Widget::updatePaths() {
    QPointF *f = points[0], *a = points[1], *b = points[2];

    //directrix
    double A = b->y() - a->y(), B = a->x() - b->x(), C = b->x() * a->y() - a->x() * b->y();
    double phi = atan(-A / B);

    //focal parameter
    double p = (A * f->x() + B * f->y() + C) / sqrt(A * A + B * B);

    //curve
    curve = QPainterPath();

    double x, y, px = 0, py = 0;
    for (double q = 0; q <= 2 * M_PI; q += 0.001 * M_PI) {
        x = ec * p / (1 - ec * cos(q - phi + (a->x() >= b->x() ? -M_PI_2 : M_PI_2))) * cos(q) + f->x();
        y = ec * p / (1 - ec * cos(q - phi + (a->x() >= b->x() ? -M_PI_2 : M_PI_2))) * sin(q) + f->y();

        if (!q || (fabs(px - x) + fabs(py - y)) / 2 > 25)
            curve.moveTo(x, y);
        else
            curve.lineTo(x, y);

        px = x;
        py = y;
    }

    //first dirextrix
    d11 = *a - 100000 * (*b - *a);
    d12 = *a + 100000 * (*b - *a);

    //center
    double c = ec * ec * p / (1 - ec * ec);
    x = (a->x() * (b->y() - a->y()) * (b->y() - a->y()) + f->x() * (b->x() - a->x()) * (b->x() - a->x()) + (b->x() - a->x()) * (b->y() - a->y()) * (f->y() - a->y())) / ((b->y() - a->y()) * (b->y() - a->y()) + (b->x() - a->x()) * (b->x() - a->x()));
    y = (b->y() - a->y()) * (x - a->x()) / (b->x() - a->x()) + a->y();

    QVector2D m(*f - QPointF(x, y));
    m = QVector2D(*f) + (p < 0 ? -c : c) * m.normalized();

    secPoints[0]->rx() = m.x();
    secPoints[0]->ry() = m.y();

    //second focus
    secPoints[1]->rx() = f->x() + 2 * (m.x() - f->x());
    secPoints[1]->ry() = f->y() + 2 * (m.y() - f->y());

    //second dirextrix
    QVector2D delta = 2 * (m - QVector2D(x, y));
    d21 = d11 + delta.toPointF();
    d22 = d12 + delta.toPointF();
}

void Widget::timerEvent(QTimerEvent *) {
    repaint();
}

void Widget::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        if (e->modifiers() & Qt::ShiftModifier)
            scale(1 / 1.9);
        else
            scale(1 / 1.2);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        if (e->modifiers() & Qt::ShiftModifier)
            scale(1.9);
        else
            scale(1.2);
        break;
    case Qt::Key_Left:
        if (e->modifiers() & Qt::ShiftModifier)
            shift(-100, 0);
        else
            shift(-10, 0);
        break;
    case Qt::Key_Right:
        if (e->modifiers() & Qt::ShiftModifier)
            shift(100, 0);
        else
            shift(10, 0);
        break;
    case Qt::Key_Up:
        if (e->modifiers() & Qt::ShiftModifier)
            shift(0, -100);
        else
            shift(0, -10);
        break;
    case Qt::Key_Down:
        if (e->modifiers() & Qt::ShiftModifier)
            shift(0, 100);
        else
            shift(0, 10);
        break;

    case Qt::Key_R:
        reset();
        break;
    case Qt::Key_A:
        aa = !aa;
        break;
    case Qt::Key_I:
        showInfo = !showInfo;
        break;
    case Qt::Key_G:
        showGrid = !showGrid;
        break;
    case Qt::Key_X:
        showAxes = !showAxes;
        break;
    case Qt::Key_N:
        showNumbers = !showNumbers;
        break;

    case Qt::Key_Q:
        if (e->modifiers() & Qt::ShiftModifier)
            ec /= 1.0510100501;
        else
            ec /= 1.01;
        updatePaths();
        break;
    case Qt::Key_W:
        if (e->modifiers() & Qt::ShiftModifier)
            ec *= 1.0510100501;
        else
            ec *= 1.01;
        updatePaths();
        break;
    case Qt::Key_F1:
    case Qt::Key_Tab:
        showHelp = true;
        break;

    case Qt::Key_F11:
        isFullScreen() ? showNormal() : showFullScreen();
        break;
    case Qt::Key_Escape:
        isFullScreen() ? showNormal() : (void)close();
        break;
    }
}

void Widget::keyReleaseEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_F1:
    case Qt::Key_Tab:
        showHelp = false;
        break;
    }
}

void Widget::mousePressEvent(QMouseEvent *e) {
    foreach (QPointF *p, points)
        if ((e->pos() - (scalef * *p + offset + rect().center())).manhattanLength() <= 5 * r) {
            moving = p;
            break;
        }

    lastPos = e->pos();
}

void Widget::mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton || e->buttons() & Qt::RightButton) {
        if (moving) {
            *moving += (e->pos() - lastPos) / scalef;
            updatePaths();
        } else
            shift(e->pos() - lastPos);
    } else {
        hover = 0;
        foreach (QPointF *p, points)
            if ((e->pos() - (scalef * *p + offset + rect().center())).manhattanLength() <= 5 * r) {
                hover = p;
                break;
            }
        foreach (QPointF *p, secPoints)
            if ((e->pos() - (scalef * *p + offset + rect().center())).manhattanLength() <= 5 * r) {
                hover = p;
                break;
            }
    }

    lastPos = e->pos();
}

void Widget::mouseReleaseEvent(QMouseEvent *) {
    moving = 0;
}

void Widget::wheelEvent(QWheelEvent *e) {
    int delta = e->delta();

    if (e->modifiers() & Qt::ControlModifier) {
        if (abs(delta) <= 600)
            if (delta >= 0)
                ec *= 1.01;
            else
                ec /= 1.01;
        else if (delta >= 0)
            ec *= 1.1;
        else
            ec *= 1.1;
        updatePaths();
    } else if (abs(delta) <= 600)
        if (delta >= 0)
            scale(1.1);
        else
            scale(1 / 1.1);
    else if (delta >= 0)
        scale(1.5);
    else
        scale(1 / 1.5);
}

void Widget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(240, 240, 240));
    int ax = -width() / 2, bx = width() / 2, ay = -height() / 2, by = height() / 2;
    p.translate(bx, by);

    //grid
    int d = scalef;
    if (showGrid && d >= 10) {
        p.setPen(QPen(Qt::lightGray, 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));

        for (int x = 0; x <= bx + d; x += d)
            p.drawLine(x + (int)offset.x() % d, -by, x + (int)offset.x() % d, by);
        for (int x = 0; x >= -bx - d; x -= d)
            p.drawLine(x + (int)offset.x() % d, -by, x + (int)offset.x() % d, by);
        for (int y = 0; y <= by + d; y += d)
            p.drawLine(-bx, y + (int)offset.y() % d, bx, y + (int)offset.y() % d);
        for (int y = 0; y >= -by - d; y -= d)
            p.drawLine(-bx, y + (int)offset.y() % d, bx, y + (int)offset.y() % d);
    }

    //axes
    if (showAxes) {
        p.setPen(Qt::darkGray);
        p.drawLine(-bx, offset.y(), bx, offset.y());
        p.drawLine(offset.x(), -by, offset.x(), by);

        if (showNumbers && d >= 20) {
            QPoint off = offset.toPoint();
            for (int x = -off.x() / d * d; x <= bx - off.x() / d * d + d; x += d)
                p.drawText(QRect(x + offset.x() - 101, offset.y(), 101, 101), Qt::AlignRight | Qt::AlignTop, QString::number(x / d));
            for (int x = -off.x() / d * d; x >= -bx - off.x() / d * d - d; x -= d)
                p.drawText(QRect(x + offset.x() - 101, offset.y(), 101, 101), Qt::AlignRight | Qt::AlignTop, QString::number(x / d));
            for (int y = -off.y() / d * d; y <= by - off.y() / d * d + d; y += d)
                p.drawText(QRect(offset.x() - 101, y + offset.y(), 101, 101), Qt::AlignRight | Qt::AlignTop, QString::number(-y / d));
            for (int y = -off.y() / d * d; y >= -by - off.y() / d * d - d; y -= d)
                p.drawText(QRect(offset.x() - 101, y + offset.y(), 101, 101), Qt::AlignRight | Qt::AlignTop, QString::number(-y / d));
        }
    }

    //antialiasing
    if (aa)
        p.setRenderHint(QPainter::Antialiasing);

    //curve
    p.save();
    p.translate(offset);
    p.scale(scalef, scalef);
    p.strokePath(curve, QPen(Qt::red, 1 / scalef));
    p.restore();

    //directrix
    p.setPen(QPen(Qt::darkGreen, 1, Qt::DashLine, Qt::RoundCap, Qt::BevelJoin));
    p.drawLine(scalef * d11 + offset, scalef * d12 + offset);
    p.drawLine(scalef * d21 + offset, scalef * d22 + offset);

    //secondary points
    QPainterPath pp;
    double x, y;
    foreach (QPointF *p, secPoints) {
        x = scalef * p->x() + offset.x();
        y = scalef * p->y() + offset.y();
        if (p == hover)
            pp.addEllipse(x - 5, y - 5, 11, 11);
        else
            pp.addEllipse(x - 3, y - 3, 7, 7);
    }

    p.fillPath(pp, QColor(255, 150, 150, 200));
    p.strokePath(pp, QColor(255, 0, 0));

    //points
    pp = QPainterPath();
    foreach (QPointF *p, points) {
        x = scalef * p->x() + offset.x();
        y = scalef * p->y() + offset.y();

        if (x - 3 > bx) {
            if (y - 3 > 2 * by / 2) {
                pp.moveTo(bx, 2 * by / 2);
                pp.lineTo(bx - 3, 2 * by / 2);
                pp.lineTo(bx, 2 * by / 2 - 3);
                pp.lineTo(bx, 2 * by / 2);
                pp.closeSubpath();
            } else if (y + 3 < -by) {
                pp.moveTo(bx, -by);
                pp.lineTo(bx - 3, -by);
                pp.lineTo(bx, -by + 3);
                pp.lineTo(bx, -by);
                pp.closeSubpath();
            } else {
                pp.moveTo(bx, y);
                pp.lineTo(bx - 3, y - 3);
                pp.lineTo(bx - 3, y + 3);
                pp.lineTo(bx, y);
                pp.closeSubpath();
            }
        } else if (x + 3 < -bx) {
            if (y - 3 > 2 * by / 2) {
                pp.moveTo(-bx, 2 * by / 2);
                pp.lineTo(-bx + 3, 2 * by / 2);
                pp.lineTo(-bx, 2 * by / 2 - 3);
                pp.lineTo(-bx, 2 * by / 2);
                pp.closeSubpath();
            } else if (y + 3 < -by) {
                pp.moveTo(-bx, -by);
                pp.lineTo(-bx + 3, -by);
                pp.lineTo(-bx, -by + 3);
                pp.lineTo(-bx, -by);
                pp.closeSubpath();
            } else {
                pp.moveTo(-bx, y);
                pp.lineTo(-bx + 3, y - 3);
                pp.lineTo(-bx + 3, y + 3);
                pp.lineTo(-bx, y);
                pp.closeSubpath();
            }
        } else if (x + 3 >= -bx && x - 3 <= bx) {
            if (y - 3 > 2 * by / 2) {
                pp.moveTo(x, 2 * by / 2);
                pp.lineTo(x - 3, 2 * by / 2 - 3);
                pp.lineTo(x + 3, 2 * by / 2 - 3);
                pp.lineTo(x, 2 * by / 2);
                pp.closeSubpath();
            } else if (y + 3 < -by) {
                pp.moveTo(x, -by);
                pp.lineTo(x - 3, -by + 3);
                pp.lineTo(x + 3, -by + 3);
                pp.lineTo(x, -by);
                pp.closeSubpath();
            } else {
                if (p == hover)
                    pp.addEllipse(x - 5, y - 5, 11, 11);
                else
                    pp.addEllipse(x - 3, y - 3, 7, 7);
            }
        }
    }

    p.fillPath(pp, QColor(150, 150, 255, 200));
    p.strokePath(pp, QColor(0, 0, 255));

    //info
    if (showInfo) {
        p.setPen(Qt::white);
        p.setFont(QFont("Lucida Console", 9));

        //up
        p.fillRect(ax, ay, 2 * bx + 1, 13, QColor(0, 0, 0, 128));

        p.drawText(ax + 1, ay + 10, tr("delta x = %1 \t delta y = %2 \t scale = %3 \t eccentricity = %4")
                                        .arg(offset.x())
                                        .arg(offset.y())
                                        .arg(scalef / 50, 0, 'f', 3)
                                        .arg(ec, 0, 'f', 3));

        p.drawText(bx - 75, ay + 10, tr("F1 - Help"));

        //down
        p.fillRect(ax, by - 13, 2 * bx + 1, 13 + 1, QColor(0, 0, 0, 128));

        p.drawText(ax + 1, by - 2, tr("%1 \t %2")
                                       .arg(ec < 1 ? "e < 1 : Ellipse" : ec > 1 ? "e > 1 : Hyperbola" : "e = 1 : Parabola")
                                       .arg(hover ? tr("(%1;%2)").arg(hover->x(), 0, 'f', 3).arg(-hover->y(), 0, 'f', 3) : ""));
    }

    //help
    if (showHelp) {
        p.setPen(Qt::white);
        p.setFont(QFont("Courier New", 10));

        p.translate(-bx, -by);
        p.fillRect(rect(), QColor(0, 0, 0, 128));

        QRect r = QRect(0, 0, bx, by).translated(rect().center() / 2);
        p.fillRect(r, QColor(0, 0, 0, 175));

        p.drawText(r, Qt::AlignCenter,
                   QString() + "Eccentricity v2.3 by Arthur Goodman" + "\n" +
                       "\n" +
                       "Use MB to move blue points" + "\n" +
                       "\n" +
                       "Plus/Minus or MW up/down - zoom in/out" + "\n" +
                       "Q/W or Ctrl+MW down/up - decrease/increase eccentricity" + "\n" +
                       "\n" +
                       "R - reset all" + "\n" +
                       "A - toggle antialiasing" + "\n" +
                       "I - toggle information" + "\n" +
                       "G - toggle grid" + "\n" +
                       "X - toggle axes" + "\n" +
                       "N - toggle numbers" + "\n" +
                       "\n" +
                       "F1 or Tab - help" + "\n" +
                       "F11 - toggle fullscreen" + "\n" +
                       "Escape - exit fullscreen or quit");
    }
}
