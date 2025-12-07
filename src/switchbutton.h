#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QAbstractButton>
#include <QColor>
#include <QMouseEvent>
#include "globals.h"

class SwitchButton : public QAbstractButton {
    Q_OBJECT
public:
    explicit SwitchButton(QWidget* parent = nullptr);

    QSize sizeHint() const override { return {40, 22}; }
    QColor activeColor;
    QColor inactiveColor;

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

private:
    bool m_checked = false;
};

#endif // SWITCHBUTTON_H
