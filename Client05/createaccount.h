#ifndef CREATEACCOUNT_H
#define CREATEACCOUNT_H

#include <QObject>
#include <QWidget>
#include <QWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include "mainwindow.h"
class CreateAccount : public QWindow
{
    Q_OBJECT
private:
    int sockDesc;
    MainWindow* parent;

    QWidget *window;

    QLabel *labelTitle;
    QLabel *labelName;
    QLabel *labelPass;
    QLabel *labelPassConfirm;

    QLabel *labelWarningName;
    QLabel *labelWarningPass;
    QLabel *labelWarningConfirmPass;

    QLayout *warningsLayout;
    QLayout * titleLayout;
    QLayout *lineEditLayout;
    QLayout *labelLayout;
    QLayout* credentialsLayout;
    QLayout* buttonsLayout;
    QLayout* generalLayout;

    QLineEdit* lineEditName;
    QLineEdit* lineEditPass;
    QLineEdit* lineEditConfirmPass;

    QPushButton *confirmButton;
    QPushButton *cancelButton;

private slots:
    void callOnDestroy();

    void onConfirmClick();

    void onCancelClick();

    void onNameModify(const QString&);

    void onPassModify(const QString&);

    void onConfirmPassModify(const QString&);
public:
    CreateAccount(MainWindow*, int commSock);
    ~CreateAccount();
};

#endif // CREATEACCOUNT_H
