#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H
#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QMessageBox>
#include "mainwindow.h"
class SettingsWindow : public QWindow
{
  Q_OBJECT
private:
    MainWindow *parent;
    QWidget *settingsWindow;
    QPushButton *cancelButton;
    QPushButton *saveButton;
    QPushButton *acceptButton;
    QLayout *generalLayout;
    QLayout *buttonsLayout;
    QLayout *optionsLayout;
    QLabel *serverInformationLabel;
    QLayout *labelLayout;
    QLayout *lineEditLayout;
    QLabel *serverIPLabel;
    QLineEdit *serverIPLineEdit;
    QLabel *serverPortLabel;
    QLineEdit *serverPortLineEdit;
private slots:

    void on_settingsPushButtonCancel_clicked();

    void on_settingsPushButtonSave_clicked();

    void on_settingsPushButtonAccept_clicked();

    void on_settingsClosed();

public:
    SettingsWindow(MainWindow* parentClass);
    ~SettingsWindow();
};

#endif // SETTINGSWINDOW_H
