#ifndef MY_WHITEBOARD_H
#define MY_WHITEBOARD_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QQueue>
#include <QTimer>

#include "sgwidget.h"
#include "server.h"

class Whiteboard : public SGWidget
{
    Q_OBJECT
public:
    explicit Whiteboard(QString name, QWidget* parent = nullptr);
    void draw_line(const QPoint& point1, const QPoint& point2, const QColor& pen_color_arg, const int& pen_size_arg);
    QByteArray* get_whiteboard();
    void update_whiteboard(QByteArray*);
    QColor get_pen_color() { return pen_color; }
    int get_pen_size() { return pen_size; }
    void set_pen_color(QColor color_arg) { pen_color = color_arg; }
    void set_pen_size(int size_arg) { pen_size = size_arg; }
    const QString& get_group_id() { return group_id; }

signals:
    void line_drawn();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void process_paints();
    void do_work();

private:
    void WBLN(QByteArray& line_info);
    void NUWB(QByteArray& user_ip);
    void WBUP(QByteArray& whiteboard_ba);
    void draw_and_send(QPoint new_position, QColor color = nullptr);
    // mouse_pos_queue [ ( (point1, point2), (color, size) ), ]
    QQueue<QPair<QPair<QPoint, QPoint>, QPair<QColor, int>>>* mouse_pos_queue;
    QString group_id;
    QPoint prev_mouse_pos;
    QImage image;
    QTimer update_timer;
    bool drawing;
    bool erasing;
    bool ruler_drawing;
    QColor pen_color;
    int pen_size;
};

#endif // MY_WHITEBOARD_H
