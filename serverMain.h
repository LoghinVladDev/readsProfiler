#pragma once
#include "Database.h"
#define PORT 3000
#define MAXTHREADS 100
#define MAXUSERS 50
//FilterFormat : char* = pageindex;noperpage;autor;titlu;anmic;anmare;categorie;rating
//pageindex = char* itoa(int),la a cata pagina e user-u
//noperpage = char* itoa(int), cate pe pagina
//autor  = char*
//titlu = char*
//anmic = char* itoa(int)
//anmare = char* itoa(int)
//categorie = char* w in {0,1}*, freqString
//rating = char* itoa(int)
// msg [in client] = sprintf(msg,"%s(itoa'd)%s(itoa'd);%s;%s;%s(itoa'd);%s(itoa'd);%s;%s(itoa'd)",pageindex, noperpage, auth, title, syear,byear,cats, rat);


enum reports{
    socketError, bindError, listenError, acceptError,
    serverPortWait, threadMessageWait, threadReadError,
    threadReadSuccess,threadWriteNotify,threadWriteError,
    threadWriteSuccess, threadLogonSuccessful, threadLogonFailCapacity,
    threadLogonFailLoggedIn
};

class Server
{
    private:
        /*
            Structure containing thread id and linked client descriptor
        */
        struct thData
        {
            int idThread;
            int clientDescriptor;
            Server* parentServer;
            char * linkedUser;
        };
        char** loggedUsers;
        int nLoggedUsers;
        int serverPort;
        int socketDescriptor;
        int pid;
        FILE* logFile{nullptr};
        pthread_t *threads;
        sockaddr_in serverInfo;
        sockaddr_in receivedInfo;
        static void* ThreadInitCall(void*);
        FILE* CreateLog();
        void ReportLog(const int = -1,const int = 0, const char* = nullptr);
        bool ThreadLoginBootstrap(void*);
        bool isUserLoggedIn(const char*);
        void ThreadRuntime(void*);
    public:
        Server(const int& = PORT);
        char *GetLocaltime();
        void Init();
        void PrintLoggedUsers();
        void Runtime();
        ~Server();
};

void Server::ThreadRuntime(void* arg)
{
    Server::thData threadDataLocal = *((thData*)arg);
    DBFilter userFilter;
    userFilter.authorName = (char*)malloc(128);
    userFilter.bookName = (char*)malloc(128);
    char *request = (char*)malloc(1024);
    char *sentInfo = (char*)malloc(65536);
    while(true)
    {
        memset(request, 0, 1024);
        if( 0 > read(threadDataLocal.clientDescriptor, request, 1024) )
            this->ReportLog(threadReadError, threadDataLocal.idThread, threadDataLocal.linkedUser);
        memset(sentInfo,0,65536);
        Database locationIndexer("./data/libIndex.json");
        printf("%s\n", request);
        locationIndexer.DecodeFilter(&userFilter, request);
        printf("Page: %d\nEntries: %d\nName: %s\nAuthor: %s\nmYear: %d\nMYear: %d\nFlags: %d\nrating: %d\n",userFilter.page, userFilter.entriesPerPage,
            userFilter.bookName, userFilter.authorName, userFilter.minYear, userFilter.maxYear, userFilter.typeFlags, userFilter.rating);
        strcpy(sentInfo, "suge-o");
        if( 0 >= write(threadDataLocal.clientDescriptor, sentInfo, 1024))
            this->ReportLog(threadWriteError, threadDataLocal.idThread, threadDataLocal.linkedUser);
    }
}

void* Server::ThreadInitCall(void* arg)
{
    Server::thData threadDataLocal;
    threadDataLocal = *((thData*)arg);
    threadDataLocal.parentServer->ReportLog(threadMessageWait, threadDataLocal.idThread);
    if(threadDataLocal.parentServer->ThreadLoginBootstrap((thData*)arg))
    {
        threadDataLocal.parentServer->ThreadRuntime((thData*)arg);
    } //TODO : investigate this crap
    pthread_detach(pthread_self()); //! detach aici sau mai jos???
    close((intptr_t)arg);
    return (NULL);

}

void Server::PrintLoggedUsers()
{
    for(size_t index = 0; index < this->nLoggedUsers; index++)
    {
        printf("%s\n", *(this->loggedUsers + index));
    }
}

bool Server::isUserLoggedIn(const char* username)
{
    for(size_t index = 0; index < this->nLoggedUsers; index++)
    {
        if(!strcmp(*(this->loggedUsers + index), username))
            return true;
    }
    return false;
}

int kbhit()
{
    timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

bool Server::ThreadLoginBootstrap(void* arg)
{
    FILE* usr, *pwd;
    int attempts = 0;
    char *clientRequest = (char*)malloc(256);
    char * username = (char*)malloc(128);
    memset(username, 0, 128);
    bool isLoggedOn = false;
    char *p;
    Server::thData threadDataLocal;
    threadDataLocal = *((Server::thData*)arg);
    do
    {
        Database usersDB("./data/accounts.json");
        memset(clientRequest, 0, 256);
        if( 0 > read(threadDataLocal.clientDescriptor, clientRequest, 256))
            this->ReportLog(threadReadError, threadDataLocal.idThread);
        else
            this->ReportLog(threadReadSuccess, threadDataLocal.idThread);
        p = strtok(clientRequest, ";");
        p = strtok(NULL, ";");
        isLoggedOn = usersDB.ConfirmLogon(clientRequest, p);
        if(!isLoggedOn)
            attempts++;
    } while (isLoggedOn == false && attempts < 3);
    strcpy(username, clientRequest);
    strcpy(clientRequest,  isLoggedOn ? "LOGONACCMPL" : "LOGONFAILED");
    if(this->nLoggedUsers == MAXUSERS && isLoggedOn)
    {
        strcpy(clientRequest, "LGERRCAPHIT");
        this->ReportLog(threadLogonFailCapacity,threadDataLocal.idThread, username);
        isLoggedOn = false;
    }
    if(this->isUserLoggedIn(username))
    {
        strcpy(clientRequest, "ERRLOGGEDIN");
        this->ReportLog(threadLogonFailLoggedIn, threadDataLocal.idThread, username);
        isLoggedOn = false;
    }
    this->ReportLog(threadWriteNotify, threadDataLocal.idThread);
    if( 0>= write(threadDataLocal.clientDescriptor, clientRequest, 11))
        this->ReportLog(threadWriteError, threadDataLocal.idThread);
    else
        this->ReportLog(threadWriteSuccess, threadDataLocal.idThread);
    if(isLoggedOn == false)
        return false;
    *(this->loggedUsers + nLoggedUsers) = (char*)malloc(128);
    memset(*(this->loggedUsers + nLoggedUsers), 0 ,128);
    strcpy(*(this->loggedUsers + nLoggedUsers++), username);
    fflush(stdout);
    this->ReportLog(threadLogonSuccessful, threadDataLocal.idThread, username);
    (*((thData*)arg)).linkedUser = (char*)malloc(128);
    strcpy((*((thData*)arg)).linkedUser, username);
    free(clientRequest);
    free(username);
    return true;
}


void Server::Runtime()
{
    int client;
    char * serverInput = (char* )malloc(128);
    memset(serverInput,0 ,128);
    unsigned int addrLen;
    int threadIndex = 0;
    Server::thData *threadData;
    while(true)
    {
        addrLen = sizeof(this->receivedInfo);
        this->ReportLog(serverPortWait);
        if( 0 > (client = accept(this->socketDescriptor, (sockaddr*)&this->receivedInfo,(socklen_t*) &addrLen)))
        {
            this->ReportLog(acceptError);
            continue;
        }
        threadData = (thData*)malloc(sizeof(thData));
        threadData->idThread = threadIndex++;
        threadData->clientDescriptor = client;
        threadData->parentServer = this;
        pthread_create(&this->threads[threadIndex], NULL, this->ThreadInitCall, threadData);
    }
}

void Server::Init()
{
    this->logFile = this->CreateLog();
    if(-1 == (this->socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)))
        this->ReportLog(socketError);
    int on;
    setsockopt(this->socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    if(-1 == bind(this->socketDescriptor, (sockaddr *)&this->serverInfo, sizeof(sockaddr)))
        this->ReportLog(bindError);
    if(-1 == listen(this->socketDescriptor, 5)) 
        this->ReportLog(listenError);
}

Server::Server(const int& givenPort)
{
    this->pid = 0;
    this->nLoggedUsers = 0;
    this->socketDescriptor = 0;
    this->serverPort = givenPort;
    this->threads = (pthread_t *)malloc(sizeof(pthread_t) * 100);
    this->loggedUsers = (char**)malloc(MAXUSERS);
    memset(this->loggedUsers, 0, MAXUSERS);
    memset(this->threads, 0, MAXTHREADS*sizeof(pthread_t));
    memset(&this->serverInfo, 0, sizeof(sockaddr_in));
    memset(&this->receivedInfo, 0, sizeof(sockaddr_in));
    this->serverInfo.sin_family = AF_INET;
    this->serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    this->serverInfo.sin_port = htons(this->serverPort);
}

Server::~Server()
{
    this->socketDescriptor = 0;
    this->pid = 0;
    this->serverPort = 0;
    free(this->threads);
    fclose(this->logFile);
    this->logFile = nullptr;
}

char* Server::GetLocaltime()
{
    static char* timeString = (char*)malloc(256);
    memset(timeString,0,256);
    time_t t = time(NULL);
    tm timeStruct = *localtime(&t);
    sprintf(timeString,"%d-%d-%d<->%d:%d:%d", timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec,
        timeStruct.tm_mday, timeStruct.tm_mon, timeStruct.tm_year);
    return timeString;
}

void Server::ReportLog(const int logId, const int threadId,const char* userId)
{
    switch(logId)
    {
        case socketError:
            fprintf(this->logFile,"[Server][%s]socket() function error!\nServer exited with code 1.\n",this->GetLocaltime());
            fflush(this->logFile);
            exit(1);         
        case bindError:
            fprintf(this->logFile,"[Server][%s]bind() function error!\nServer exited with code 2.\n",this->GetLocaltime());
            fflush(this->logFile);
            exit(2);
        case listenError:
            fprintf(this->logFile,"[Server][%s]listen() function error!\nServer exited with code 3.\n",this->GetLocaltime());
            fflush(this->logFile);
            exit(3);
        case acceptError:
            fprintf(this->logFile,"[Server][%s]accept() function error!\nContinuing Server program.\n",this->GetLocaltime());
            break;
        case serverPortWait:
            fprintf(this->logFile, "[Server][%s]Waiting @ port %d...\n", this->GetLocaltime(), this->serverPort);
            break;
        case threadMessageWait:
            fprintf(this->logFile, "[Thread - %d][%s]Waiting for message...\n", threadId, this->GetLocaltime());
            break;
        case threadReadError:
            fprintf(this->logFile,"[Thread - %d][%s]read() function error!\nContinuing program.\n",threadId, this->GetLocaltime());
            break;
        case threadWriteError:
            fprintf(this->logFile,"[Thread - %d][%s]write() function error!\nContinuing program.\n",threadId, this->GetLocaltime());
            break;
        case threadWriteNotify:
            fprintf(this->logFile,"[Thread - %d][%s]Writing message to client.\n",threadId, this->GetLocaltime());
            break;
        case threadWriteSuccess:
            fprintf(this->logFile,"[Thread - %d][%s]Wrote message to client.\n",threadId, this->GetLocaltime());
            break;
        case threadReadSuccess:
            fprintf(this->logFile,"[Thread - %d][%s]Received message from client.\n",threadId, this->GetLocaltime());
            break;
        case threadLogonFailCapacity:
            fprintf(this->logFile,"[Thread - %d][%s]Users capacity reached!\nUser %s failed to log on.\n", threadId, this->GetLocaltime(), userId);
            break;
        case threadLogonFailLoggedIn:
            fprintf(this->logFile,"[Thread - %d][%s]User %s already logged in!\n", threadId, this->GetLocaltime(), userId);
            break;
        case threadLogonSuccessful:
            fprintf(this->logFile,"[Thread - %d][%s]User %s has logged into the server!\n", threadId, this->GetLocaltime(), userId);
            break;
        default:
            fprintf(this->logFile,"[Server][%s]Unknown Log Request!!\n",this->GetLocaltime());
            fflush(this->logFile);
            return;
    }
    fflush(this->logFile);
}

FILE* Server::CreateLog()
{
    static FILE* fileName;
    char* logName = (char*)malloc(256);
    memset(logName, 0, 256);
    sprintf(logName,"./logs/server/log:%s", this->GetLocaltime());
    fileName = fopen(logName, "w");
    free(logName);
    return fileName;
}
