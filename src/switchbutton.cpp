#include "switchbutton.h"
#include <QPainter>
#include <QMouseEvent>

SwitchButton::SwitchButton(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);

    activeColor = globals::isDark
                      ? QColor::fromRgb(0x0094ff)
                      : QColor::fromRgb(0x0078d7);

    inactiveColor = globals::isDark
                        ? QColor::fromRgb(0x777777)
                        : QColor::fromRgb(0xcccccc);
}

void SwitchButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int r = h / 2;

    // fundal
    p.setBrush(isChecked() ? activeColor : inactiveColor);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(0, 0, w, h, r, r);

    // cerc
    int circleX = isChecked() ? (w - h) : 0;

    p.setBrush(Qt::white);
    p.drawEllipse(circleX, 0, h, h);
}

void SwitchButton::mousePressEvent(QMouseEvent* e)
{
    QAbstractButton::mousePressEvent(e);
}
