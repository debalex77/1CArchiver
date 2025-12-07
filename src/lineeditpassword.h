#ifndef LINEEDITPASSWORD_H
#define LINEEDITPASSWORD_H

#include <QLineEdit>

class LineEditPassword : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEditPassword(QWidget *parent = nullptr);
    ~LineEditPassword();

private slots:
    void onClickedButton();

private:
    bool show_passwd;
    QAction *action_hideShowPassword;
};

#endif // LINEEDITPASSWORD_H
