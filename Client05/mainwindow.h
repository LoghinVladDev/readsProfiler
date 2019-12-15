#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include <QMainWindow>
#include <QSignalMapper>

#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The DBResult struct.
 *
 * Same as in the database header.

 * Structure which describes/contains an entry in a query result
 *
 * (char[]) bookName = the title of the book
 * (char[]) authorName = the name of the author of the book
 * (int)    year = the year in which the book was published
 * (char[]) genre = the name of the genre of the book
 * (int)    rating = the rating of the book (i out of 5)
 * (int)    ISBN = the ISBN of the book (international standard book number)
 * (char[]) diskPath = the absolute path of the file containing the book contents on the disk
 */
struct DBResult
{
    //the title of the book
    char bookName[128];
    //the name of the author of the book
    char authorName[128];
    //the year in which the book was published
    int year;
    //the name of the genre of the book
    char genre[64];
    //the rating of the book (i out of 5)
    int rating;
    //the ISBN of the book (international standard book number)
    int ISBN;
    //the absolute path of the file containing the book contents on the disk
    char diskPath[256];
};

/**
 * @brief The logs enum.
 *
 * Used to give an error code to each reported error. Improves readability.
 */
enum logs
{
    socketError, connectError, writeError, readError, connectionLost
};

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * @brief MainWindow Constructor
     * @param parent is the main application
     */
    MainWindow(QWidget *parent = nullptr);

    /**
      * @brief MainWindow Destructor
    */
    ~MainWindow();

    /**
     * @brief Client initialisation function
     */
    void Init();

    /**
     * @brief LoginRequest function
     *
     * Discontinued
     */
    void LoginRequest();

    /**
     * @brief Library interface initialisation function.
     *
     * Sets the filter interface objects to defaults.
     * Sends a message to the server containing a filter configured to show all entries
     */
    void InitLibrary();

    /**
     * @brief closeEvent
     *
     * Used to treat the event caused by the user exiting the program forcefully
     *
     * Sends the Server an exit message.
     */
    void closeEvent(QCloseEvent*);

    /**
     * @brief Runtime function
     *
     * Discontinued
     */
    void Runtime();

    /**
     * @brief PrintResults function
     *
     * [Optional]
     * [Debug function]
     * Prints the query results received from the server on the debug console
     *
     * @param DBResult* contains the address to the start of the DBResult array
     * @param int contains the length of the array
     */
    void PrintResults(DBResult*, const int&);

    /**
     * @brief GetLocalTime function
     *
     * Returns the local time and date of the client.
     *
     * Used in CreateLog(), ReportLog()...
     *
     * Time and date is stored in a static string in the following format:
     * [hour]-[minute][second]<->[day]:[month]:[year]
     *
     * @return Returns an address to the static string containing the time and date in string format
     */
    char *GetLocalTime();

signals:



private slots:

    /**
     * @brief on_pushButtonLogin_clicked
     *
     * Function is triggered when the user clicks on the Login button
     *
     * The function converts the text from the editText objects that handle the username and the password
     * into a char*;char* format to send it to the server.
     * Upon function finish, the groupBoxLogin is hidden, and the main library interface group is shown
     */
    void on_pushButtonLogin_clicked();

    /**
     * @brief on_pushButtonApplyFilter_clicked
     *
     * Function is triggered when the user clicks on the Filter Results button
     *
     * The function converts the information from the filter group objects into a char* and passes
     * it on to the server through the communication channel. Afterwards, it prints the results through
     * the AddToList() function.
     */
    void on_pushButtonApplyFilter_clicked();

    /**
     * @brief on_dateEditMin_userDateChanged
     *
     * Function is triggered whenever the user changes the minimum date on the dateEdit dial
     *
     * Also resets the dateEdit dial if the user goes over the dateMax dial or slider.
     * @param the date assigned to the dateEditMin dial
     */
    void on_dateEditMin_userDateChanged(const QDate &date);

    /**
     * @brief on_sliderMinYear_sliderMoved
     *
     * Function is triggered whenever the user changes the minimum date on the dateSlider slider
     *
     * Also resets the slider if the user goes over the Max slider or dial
     * @param position assigned to the slider
     */


    void buttonDownload_clicked(int index);

    void on_sliderMinYear_sliderMoved(int position);

    /**
     * @brief on_sliderMaxYear_sliderMoved
     *
     * Function is triggered whenever the user changes the maximum date on the dateSlider slider
     *
     * Also resets the slider if the user goes below the Min slider or dial
     * @param position assigned to the slider
     */
    void on_sliderMaxYear_sliderMoved(int position);

    /**
     * @brief on_dateEditMax_dateChanged
     *
     * Function is triggered whenever the user changes the maximum date on the dateEdit dial
     *
     * Also resets the dial if the user goes below the Min slider or dial
     * @param date
     */
    void on_dateEditMax_dateChanged(const QDate &date);

    /**
      *Discontinued
      */
   // void on_sliderMinYear_valueChanged(int value);

    /**
      *Discontinued
      */
    //void on_sliderMaxYear_valueChanged(int value);


    void on_actionDisconnect_triggered();

    void on_actionSettings_triggered();

private:
    /**
     * @brief ui
     *
     * The ui that is linked to the Client object
     */
    Ui::MainWindow *ui;

    DBResult* entries;

    int actualNumberEntries;

    QSignalMapper *signalMapper;

    int pressedButton;

    bool isConnected{false};

    //bool variable determining whether the user has logged on or not.
    bool isLoggedOn{false};

    //counter of attempts, locally stored. //TODO
    int loginAttempts;

    //descriptor of the communication channel between the client and the server.
    int socketDescriptor;

    //buffer containing the filter information that is sent to the server.
    char *filterBuffer;

    //address to the log file created by the client.
    FILE* logFile{nullptr};

    char *downloadDirPath;

    DIR* downloadDir;

    //externally declared structure containing the information required to connect to the server. Contains IP, port and protocol family of the server
    sockaddr_in serverInfo;


    /**
     * @brief GetCurrentGenreFlag
     *
     * Function returns the coded flag (base 2) associated with the genres selected from the available gernes in the filter group
     *
     * From Server/Database.h :
     *
     *      genres are stored in a flag which is in base 2, i.e.
     *      - user is looking for both thriller and horror books, thriller flag is 4 (100), horror flag is 8 (1000), therefore
     *      - what he is looking for can be described as 12 (1100)
     *
     * @return returns the flag that contains the requested genres
     */
    int GetCurrentGenreFlag();

    /**
     * @brief AddToList
     *
     * Function updates the table in which the query result will be shown.
     *
     * Converts from DBResult* array items to QTableViewItems
     *
     * @param DBResult* the array through which the query results are received.
     * @param int the length of the array
     */
    void AddToList(DBResult*, const int&);

    /**
     * @brief ReportLog
     *
     * [Optional]
     * [Requires CreateLog()]
     * Function writes a log to the created log file, log id will be passed down through the parameter
     *
     * @param idLog type int will contain an item from enum reports (refer reports), descibing the event which is to be logged
     */
    void ReportLog(const int = -1);

    /**
     * @brief CreateLog
     *
     * [Optional]
     *
     * Creates a log at the specified path (inside the function definition) TODO : get path from outside the function
     * Log is created with the following name
     * log:[hour]-[minute]-[second]<->[day]:[month]:[year].txt
     *
     * @return address of the file
     */
    FILE* CreateLog();
};
#endif // MAINWINDOW_H
