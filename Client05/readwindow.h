#ifndef READWINDOW_H
#define READWINDOW_H

#include "mainwindow.h"
#include <QWindow>
#include <QTextBrowser>
#include <QLayout>
#include <QTableWidget>

#include <QTextDocument>

class ReadWindow : public QWindow
{

   Q_OBJECT

private:
    QTextBrowser *bookTextBrowser;

    QWidget* window;
    QLayout* generalLayout;
    QLayout* browserLayout;
    QLayout* buttonsLayout;

    QPushButton* closeButton;

    MainWindow *parent;

    void addToReadWindow(const char*);

    bool implementationStatus(const char*);

    void errorBox(const char *);

private slots:
    void on_readWindowClosed();

    void closeButton_onClicked();

public:

    ReadWindow(MainWindow*, const char*);
    ~ReadWindow();
};

#endif // READWINDOW_H
