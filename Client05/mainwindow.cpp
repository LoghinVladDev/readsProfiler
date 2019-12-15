#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QSlider>
#include <QDate>
#include <QDateEdit>
#include <QCheckBox>
#include <QCloseEvent>
#include <QMessageBox>

#define DB_DRAMA 1
#define DB_COMED 2
#define DB_THRIL 4
#define DB_HORRO 8
#define DB_MANUA 16
#define DB_BIOGR 32
#define DB_ABIOG 64
#define DB_CHILD 128
#define DB_ACTIO 256
#define DB_ALLGN 511

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    memset(&this->serverInfo, 0 ,sizeof(sockaddr_in));
    this->entries = nullptr;
    this->serverInfo.sin_family = AF_INET;
    this->serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    this->serverInfo.sin_port = htons(3000);
    this->ui->setupUi(this);
    this->ui->actionDisconnect->setEnabled(false);
    this->ui->libGroup->hide();
    this->pressedButton = 0;
    this->downloadDirPath = (char*)malloc(256);
    this->signalMapper = new QSignalMapper(this);
    connect(this->signalMapper, SIGNAL(mapped(int)), this, SLOT(buttonDownload_clicked(int)));
    memset(this->downloadDirPath, 0 ,256);
    strcpy(this->downloadDirPath, "./downloads");
    if(0 == (this->downloadDir = opendir(this->downloadDirPath)))
    {
        mkdir(this->downloadDirPath, 0666);
    }
    closedir(this->downloadDir);
}

void MainWindow::Init()
{
    this->loginAttempts = 3;
    this->logFile = this->CreateLog();
    if( -1 == (this->socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)))
        this->ReportLog(socketError);
}

MainWindow::~MainWindow()
{
    ::close(this->socketDescriptor);
    fflush(this->logFile);
    this->socketDescriptor = 0;
    memset(&this->serverInfo, 0 , sizeof(sockaddr_in));
    fclose(this->logFile);
    this->logFile = nullptr;
    delete ui;
}


void MainWindow::on_pushButtonLogin_clicked()
{
    if(isConnected == false)
    {
        if( -1 == ::connect(this->socketDescriptor, (sockaddr* )&this->serverInfo, sizeof(sockaddr))) // ::connect  (=) extern::connect
        {
            this->ReportLog(connectError);
            QMessageBox *connectFailedBox = new QMessageBox();
            connectFailedBox->setText("Connection failed!");
            connectFailedBox->setInformativeText("Sever might be down. Press Login to try again");
            connectFailedBox->setStandardButtons(QMessageBox::Ok);
            connectFailedBox->setDefaultButton(QMessageBox::Ok);
            connectFailedBox->exec();
            delete connectFailedBox;
            return;
        }
        else
        {
            isConnected = true;
        }
    }
    QString username = ui->lineEditUsername->text();
    QString password = ui->lineEditPassword->text();
    char* buffer = (char*)malloc(256);
    memset(buffer, 0 , 256);
    sprintf(buffer, "%s;%s",username.toStdString().c_str(), password.toStdString().c_str());
    //printf("%s;%s;\n", username.toStdString().c_str(), password.toStdString().c_str());
    fflush(stdout);
    write(socketDescriptor, buffer, 256);
    read(socketDescriptor, buffer, 12);
    printf("Message rec : %s\n", buffer);
    fflush(stdout);
    if(!strcmp(buffer, "LOGONACCMPL"))
    {
        ui->loginGroup->hide();
        this->InitLibrary();
    }
    else
        this->loginAttempts--;
    if(!strcmp(buffer, "ERRTIMEDOUT"))
    {
        int secondsLeft = 0; char* message = (char*)malloc(128);
        read(socketDescriptor, &secondsLeft, sizeof(int));
        QMessageBox *timedoutBox = new QMessageBox();
        timedoutBox->setText("You are timed out!");
        sprintf(message, "Seconds left : %d", secondsLeft);
        timedoutBox->setInformativeText(QString(message));
        printf("Secs : %d\n", secondsLeft);
        fflush(stdout);
        free(message);
        timedoutBox->setStandardButtons(QMessageBox::Ok);
        timedoutBox->setDefaultButton(QMessageBox::Ok);
        timedoutBox->exec();
        delete  timedoutBox;
    }
    if(!strcmp(buffer, "ERRLOGGEDIN"))
    {
        QMessageBox *alreadyLoggedInBox = new QMessageBox();
        alreadyLoggedInBox->setText(QString("User " + ui->lineEditUsername->text() + " is alreay logged in!"));
        alreadyLoggedInBox->setStandardButtons(QMessageBox::Ok);
        alreadyLoggedInBox->setDefaultButton(QMessageBox::Ok);
        alreadyLoggedInBox->exec();
        delete alreadyLoggedInBox;
    }
    if(!strcmp(buffer, "LGERRCAPHIT"))
    {
        QMessageBox *capacityHitBox = new QMessageBox();
        capacityHitBox->setText(QString("Too many users logged in!"));
        capacityHitBox->setStandardButtons(QMessageBox::Ok);
        capacityHitBox->setDefaultButton(QMessageBox::Ok);
        capacityHitBox->exec();
        delete capacityHitBox;
    }
    if(!strcmp(buffer, "LOGONFAILED"))
    {
        QMessageBox *loginFailedBox = new QMessageBox();
        loginFailedBox->setText("Username / Password incorrect");
        loginFailedBox->setInformativeText("Please try again");
        loginFailedBox->setStandardButtons(QMessageBox::Ok);
        loginFailedBox->setDefaultButton(QMessageBox::Ok);
        loginFailedBox->exec();
        delete loginFailedBox;
    }
    if(!strcmp(buffer, "LOGONACCMPL"))
    {
        QMessageBox *loginSuccessBox = new QMessageBox();
        loginSuccessBox->setText("Login successful!");
        loginSuccessBox->setInformativeText("Press Ok to continue.");
        loginSuccessBox->setStandardButtons(QMessageBox::Ok);
        loginSuccessBox->setDefaultButton(QMessageBox::Ok);
        loginSuccessBox->exec();
        delete loginSuccessBox;
    }
    ui->pushButtonLogin->update();
    fflush(stdout);
}

void MainWindow::PrintResults(DBResult* array, const int& length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("Title: %s\nAuthor: %s\nGenre: %s\nYear: %d\nRating: %d\nISBN: %d\n",
            array[i].bookName, array[i].authorName, array[i].genre, array[i].year, array[i].rating,
            array[i].ISBN);
    }
    fflush(stdout);
}

char *itoa(int n)
{
    int l = 0;
    static char* result = nullptr;
    if(result != nullptr)
        free(result);
    result = (char*)malloc(50);
    char* reversed = (char*)malloc(50);
    memset(result, 0, 50);
    memset(reversed, 0 ,50);
    if(n == 0)
    {
        *result = '0';
        *(result+1) = 0;
        return result;
    }
    while(n)
    {
        reversed[l++] = n%10 + '0';
        n/=10;
    }
    for(int i = 0; i < l; i++)
        result[i] = reversed[l-i-1];
    free(reversed);
    return result;
}

void MainWindow::AddToList(DBResult * array, const int & length)
{
    ui->tableWidgetQueryResult->setRowCount(length);
    for(int i = 0; i < length; i++)
    {/*
        QTableWidgetItem** tableArray= (QTableWidgetItem**)malloc(sizeof(size_t)* 6);
        tableArray[0] = new QTableWidgetItem(QString(itoa(array[i].ISBN)));
        tableArray[1] = new QTableWidgetItem(QString(array[i].bookName));
        tableArray[2] = new QTableWidgetItem(QString(array[i].authorName));
        tableArray[3] = new QTableWidgetItem(QString(array[i].genre));
        tableArray[4] = new QTableWidgetItem(QString(itoa(array[i].year)));
        tableArray[5] = new QTableWidgetItem(QString(itoa(array[i].rating)));*/

        ui->tableWidgetQueryResult->setItem(i, 0, new QTableWidgetItem(QString(itoa(array[i].ISBN))));
        ui->tableWidgetQueryResult->setItem(i, 1, new QTableWidgetItem(QString(array[i].bookName)));
        ui->tableWidgetQueryResult->setItem(i, 2, new QTableWidgetItem(QString(array[i].authorName)));
        ui->tableWidgetQueryResult->setItem(i, 3, new QTableWidgetItem(QString(array[i].genre)));
        ui->tableWidgetQueryResult->setItem(i, 4, new QTableWidgetItem(QString(itoa(array[i].year))));
        ui->tableWidgetQueryResult->setItem(i, 5, new QTableWidgetItem(QString(itoa(array[i].rating))));



        QWidget * pWidget = new QWidget();
        QPushButton* buttonDownload = new QPushButton();
        buttonDownload->setText("Download");
        QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
        pLayout->addWidget(buttonDownload);
        pLayout->setAlignment(Qt::AlignCenter);
        pLayout->setContentsMargins(0,0,0,0);
        pWidget->setLayout(pLayout);

        connect(buttonDownload, SIGNAL(clicked()), this->signalMapper, SLOT(map()));
        this->signalMapper->setMapping(buttonDownload, i);

        ui->tableWidgetQueryResult->setCellWidget(i, 6, pWidget);
        /*for(size_t j = 0; j< 6; j++)
        {
            ui->tableWidgetQueryResult->setItem(i, j, tableArray[j]);
        }*/
        //free(tableArray);
    }
}


void MainWindow::buttonDownload_clicked(int index)
{
   /* QPushButton* button = qobject_cast<QPushButton*>(sender());
    if(!button)
        return;*/
    printf("%d\n",index);
    fflush(stdout);
    this->filterBuffer = (char*)malloc(1024);

    memset(this->filterBuffer, 0 ,1024);
    strcpy(this->filterBuffer, "CLIENTEVENTDOWN");
    write(socketDescriptor, this->filterBuffer, 1024);

    write(socketDescriptor, &index, 4);
    char *downloadFileName = (char*)malloc(256);
    memset(downloadFileName, 0 ,256);
    read(socketDescriptor, this->filterBuffer, 1024);
    strcpy(downloadFileName, this->downloadDirPath);
    strcat(downloadFileName, "/");
    strcat(downloadFileName, this->filterBuffer);
    int bookDescriptor = open(downloadFileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    long long fileSize = 0;

    printf("%s\n", downloadFileName);
    fflush(stdout);
    read(socketDescriptor, &fileSize, 8);
    int lbytes = 0;
    while((lbytes = read(socketDescriptor, this->filterBuffer, 1024)))
    {
        printf("plm");
        fflush(stdout);
        if(*this->filterBuffer == 0)
            break;
        write(bookDescriptor, this->filterBuffer, strlen(this->filterBuffer));
    }
    printf("%s\n", downloadFileName);
    fflush(stdout);

    ::close(bookDescriptor);

    free(downloadFileName);

    free(this->filterBuffer);
}

void MainWindow::InitLibrary()
{
    this->ui->actionDisconnect->setEnabled(true);
    this->filterBuffer = (char*)malloc(1024);

    memset(this->filterBuffer, 0 , 1024);
    strcpy(this->filterBuffer, "CLIENTEVENTQUER");
    write(socketDescriptor, this->filterBuffer, 1024);

    memset(this->filterBuffer, 0, 1024);
    sprintf(this->filterBuffer, "%d;%d;%s;%s;%d;%d;%d;%d",1,20,ui->lineEditAuthor->text().toStdString().c_str(),
            ui->lineEditTitle->text().toStdString().c_str(), ui->dateEditMin->minimumDate().year(),
            ui->dateEditMax->maximumDate().year(), DB_ALLGN, 0);

    write(socketDescriptor, this->filterBuffer, 1024);
    QStringList pageIndex;
    QString buffer;
    for (int i = 1; i<= 50; i++)
    {
        if(i<10)
            buffer.insert(0, (char)i+'0');
        else
        {
            buffer.insert(0, (char)i/10+'0');
            buffer.insert(1, (char)i%10 + '0');
        }
        pageIndex.push_back(QString(buffer));
        buffer.clear();
    }
    ui->comboBoxPage->addItems(pageIndex);

    ui->comboBoxItemsPage->addItem(QString("10"));
    ui->comboBoxItemsPage->addItem(QString("20"));
    ui->comboBoxItemsPage->addItem(QString("50"));

    ui->dateEditMin->setDate(QDate(1970,1,1));
    ui->dateEditMax->setDate(QDate::currentDate());

    ui->spinBoxRating->setRange(1,5);
    ui->spinBoxRating->setValue(1);


    ui->checkBoxAction->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxBio->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxAutobio->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxChildren->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxComedy->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxDrama->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxHorror->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxManual->setCheckState(Qt::CheckState::Unchecked);
    ui->checkBoxThriller->setCheckState(Qt::CheckState::Unchecked);

    ui->libGroup->show();

    this->actualNumberEntries = 0;
    this->entries = (DBResult*)malloc(sizeof(DBResult)*20);
    memset(entries, 0 ,sizeof(DBResult)*20);
    if( 0 > read(socketDescriptor, this->entries, sizeof(DBResult) * 20))
        this->ReportLog(readError);

    if( 0 > read(socketDescriptor, &this->actualNumberEntries, 4))
        this->ReportLog(readError);

    this->PrintResults(this->entries, this->actualNumberEntries);
    this->AddToList(this->entries, this->actualNumberEntries);
}

int MainWindow::GetCurrentGenreFlag()
{
    int flag = 0;
    if(ui->checkBoxBio->isChecked())
        flag = flag | DB_BIOGR;
    if(ui->checkBoxDrama->isChecked())
        flag = flag | DB_DRAMA;
    if(ui->checkBoxAction->isChecked())
        flag = flag | DB_ACTIO;
    if(ui->checkBoxComedy->isChecked())
        flag = flag | DB_COMED;
    if(ui->checkBoxHorror->isChecked())
        flag = flag | DB_HORRO;
    if(ui->checkBoxManual->isChecked())
        flag = flag | DB_MANUA;
    if(ui->checkBoxAutobio->isChecked())
        flag = flag | DB_ABIOG;
    if(ui->checkBoxChildren->isChecked())
        flag = flag | DB_CHILD;
    if(ui->checkBoxThriller->isChecked())
        flag = flag | DB_THRIL;
    if(flag == 0)
        flag = DB_ALLGN;
    return flag;
}

void MainWindow::on_pushButtonApplyFilter_clicked()
{
    int flags = GetCurrentGenreFlag();
    int bytesRead = 0;

    this->filterBuffer=(char*)malloc(1024);

    memset(this->filterBuffer, 0 , 1024);
    strcpy(this->filterBuffer, "CLIENTEVENTQUER");
    write(this->socketDescriptor, this->filterBuffer, 1024);
    memset(this->filterBuffer, 0 , 1024);
    sprintf(this->filterBuffer, "%d;%d;%s;%s;%d;%d;%d;%d", atoi(ui->comboBoxPage->currentText().toStdString().c_str()), atoi(ui->comboBoxItemsPage->currentText().toStdString().c_str()),
            ui->lineEditAuthor->text().toStdString().c_str(), ui->lineEditTitle->text().toStdString().c_str(), ui->dateEditMin->date().year(), ui->dateEditMax->date().year(),
            flags, ui->spinBoxRating->value());
    write(this->socketDescriptor, this->filterBuffer, 1024);
    this->actualNumberEntries = 0;
    int maxNumberEntries = atoi(ui->comboBoxItemsPage->currentText().toStdString().c_str());
    free(this->entries);
    this->entries = (DBResult*)malloc(sizeof(DBResult)*maxNumberEntries);
    memset(this->entries, 0 ,sizeof(DBResult)*maxNumberEntries);
    if( 0 >= (bytesRead = read(socketDescriptor, this->entries, sizeof(DBResult) * maxNumberEntries)))
        if(bytesRead < 0)
            this->ReportLog(readError);
        else
            this->ReportLog(connectionLost);

    if( 0 > read(socketDescriptor, &actualNumberEntries, 4))
        this->ReportLog(readError);
    printf("PLMCOAIE%d\n", this->actualNumberEntries);
    perror("a");
    fflush(stdout);
    this->PrintResults(this->entries, actualNumberEntries);
    perror("a");
    printf("PLMCOAIE%d\n", this->actualNumberEntries);
    perror("a");
    fflush(stdout);
    //ui->tableWidgetQueryResult->clear();
    perror("a");
    fflush(stdout);
    AddToList(this->entries, actualNumberEntries);
    perror("a");
    fflush(stdout);
    free(this->filterBuffer);
}


char* MainWindow::GetLocalTime()
{
    static char* timeString = (char*)malloc(256);
    memset(timeString,0,256);
    time_t t = time(NULL);
    tm timeStruct = *localtime(&t);
    sprintf(timeString,"%d-%d-%d<->%d:%d:%d", timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec,
        timeStruct.tm_mday, timeStruct.tm_mon, timeStruct.tm_year);
    return timeString;
}

void MainWindow::ReportLog(const int logId)
{
    switch(logId)
    {
        case socketError:
            fprintf(this->logFile,"[Client][%s]socket()function error.\nApplication exited with code 2.\n",this->GetLocalTime());
            ::exit(2);
        case connectError:
            fprintf(this->logFile,"[Client][%s]connect() function error.\nProgram exited with code 3.\n", this->GetLocalTime());
            break;
        case writeError:
            fprintf(this->logFile,"[Client][%s]write() function error.\nContinuing program.\n",this->GetLocalTime());
            break;
        case readError:
            fprintf(this->logFile,"[Client][%s]read() function error.\nContinuing program.\n",this->GetLocalTime());
            break;
        case connectionLost:
            fprintf(this->logFile,"[Client][%s]Lost connection to the server.\n", this->GetLocalTime());
            break;
        default:
            fprintf(this->logFile,"[Client][%s]Unknown Log Request!!\n",this->GetLocalTime());
            return;
    }
}

FILE* MainWindow::CreateLog()
{
    static FILE* fileName;
    char* logName = (char*)malloc(256);
    memset(logName, 0, 256);
    struct stat st;
    if(stat("./logs", &st) == -1)
        mkdir("./logs", 0700);
    sprintf(logName,"./logs/log:%s", this->GetLocalTime());
    fileName = fopen(logName, "w");
    free(logName);
    return fileName;
}

void MainWindow::on_dateEditMin_userDateChanged(const QDate &date)
{
    if(date.year() > ui->dateEditMax->date().year())
        ui->dateEditMin->setDate(QDate(ui->dateEditMin->minimumDate()));
    ui->sliderMinYear->setValue(date.year());
    ui->sliderMinYear->setSliderPosition(date.year());
}

void MainWindow::on_sliderMinYear_sliderMoved(int position)
{
    if(position > ui->sliderMaxYear->value())
        ui->sliderMinYear->setValue(ui->sliderMinYear->minimum());
    ui->dateEditMin->setDate(QDate(ui->sliderMinYear->sliderPosition(),1,1));
}

void MainWindow::on_sliderMaxYear_sliderMoved(int position)
{
    if(position < ui->sliderMinYear->value())
        ui->sliderMaxYear->setValue(ui->sliderMaxYear->maximum());
    ui->dateEditMax->setDate(QDate(ui->sliderMaxYear->sliderPosition(),1,1));
}

void MainWindow::on_dateEditMax_dateChanged(const QDate &date)
{
    if(date.year() < ui->dateEditMin->date().year())
        ui->dateEditMax->setDate(QDate(ui->dateEditMax->maximumDate()));
    ui->sliderMaxYear->setValue(date.year());
    ui->sliderMaxYear->setSliderPosition(date.year());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    char *buffer = (char*)malloc(1024);
    memset(buffer,0 ,1024);
    strcpy(buffer, "CLIENTEVENTQUIT");
    write(socketDescriptor,buffer, 1024);
    free(buffer);
    ::close(socketDescriptor);
}

void MainWindow::on_actionDisconnect_triggered()
{
    char *buffer = (char*)malloc(1024);
    memset(buffer, 0, 1024);
    strcpy(buffer, "CLIENTEVENTDISC");
    write(socketDescriptor, buffer, 1024);
    free(buffer);
    ::close(socketDescriptor);
    qApp->exit();
    //this->~MainWindow();
}

void MainWindow::on_actionSettings_triggered()
{
    QWidget *settingsWindow = new QWidget();
    QPushButton *cancelButton = new QPushButton();
    QPushButton *saveButton = new QPushButton();
    QPushButton *acceptButton = new QPushButton();
    QLayout *generalLayout = new QVBoxLayout(settingsWindow);
    QLayout *buttonsLayout = new QHBoxLayout();
    QLayout *optionsLayout = new QVBoxLayout();

    generalLayout->addItem(optionsLayout);
    generalLayout->addItem(buttonsLayout);
    optionsLayout->setParent(generalLayout);
    buttonsLayout->setParent(generalLayout);

    cancelButton->setText("Cancel");
    saveButton->setText("Save");
    acceptButton->setText("Accept");

    buttonsLayout->setAlignment(Qt::AlignBottom);
    buttonsLayout->setContentsMargins(0,0,0,0);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addWidget(acceptButton);

    QLabel *serverInformationLabel = new QLabel();
    QLayout *IPLayout = new QHBoxLayout();
    QLabel *serverIPLabel = new QLabel();
    QLineEdit *serverIPLineEdit = new QLineEdit();
    QLayout *PortLayout = new QHBoxLayout();
    QLabel *serverPortLabel = new QLabel();
    QLineEdit *serverPortLineEdit = new QLineEdit();

    settingsWindow->setLayout(buttonsLayout);

    settingsWindow->show();
}

/*
void MainWindow::on_sliderMinYear_valueChanged(int value)
{
    if(value > ui->sliderMaxYear->value())
        ui->sliderMinYear->setValue(ui->sliderMinYear->minimum());
    ui->dateEditMin->setDate(QDate(value,0,0));
}

void MainWindow::on_sliderMaxYear_valueChanged(int value)
{
    if(value < ui->sliderMinYear->value())
        ui->sliderMaxYear->setValue(ui->sliderMaxYear->maximum());
    ui->dateEditMax->setDate(QDate(value,0,0));
}
*/

