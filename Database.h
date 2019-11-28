#pragma once
#include <json-c/json.h>
#include <stdio.h>
#include <cstring>

#define MAXBUFFERSIZE 8192
#define _DB_DRAMA 1
#define _DB_COMED 2
#define _DB_THRIL 4
#define _DB_HORRO 8
#define _DB_MANUA 16
#define _DB_BIOGR 32
#define _DB_ABIOG 64
#define _DB_CHILD 128
#define _DB_ALLGN 255

struct DBFilter{
    int page;
    int entriesPerPage;
    char *bookName;
    char *authorName;
    int minYear;
    int maxYear;
    int typeFlags;
    int rating;
};

struct DBResult
{
    char *bookName;
    char *authorName;
    int year;
    int genre;
    int rating;
    int ISBN;
    char* diskPath;
};

class Database
{
    private:
        FILE* dbDescriptor{nullptr};
        json_object *parsed_json{nullptr};
        char* buffer{nullptr};
    public:
        /*
            Argument is database relative path.
        */
        Database(const char*);
        void DecodeFilter(DBFilter *, const char*);
        bool ConfirmLogon(const char*, const char *);
        ~Database();
};

void Database::DecodeFilter(DBFilter* givenFilter, const char* givenInfo)
{
    char* infoCopy = (char*)malloc(1024), *p;
    memset(givenFilter->authorName,0 ,128);
    memset(givenFilter->bookName, 0 ,1024);
    givenFilter->entriesPerPage = 20;
    givenFilter->page = 1;
    givenFilter->minYear = -1;
    givenFilter->maxYear = -1;
    givenFilter->rating = -1;
    givenFilter->typeFlags = _DB_ALLGN;
    memset(infoCopy, 0, 1024);
    strcpy(infoCopy, givenInfo);
    p = infoCopy;
    *strchr(p, ';') = 0;
    if(*p)
        givenFilter->page = atoi(p);
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    if(*p)
        givenFilter->entriesPerPage = atoi(p);
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    if(*p)
        strcpy(givenFilter->authorName, p);   
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    if(*p)
        strcpy(givenFilter->bookName, p);  
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    if(*p)
        givenFilter->minYear = atoi(p); 
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    if(*p)
        givenFilter->maxYear = atoi(p); 
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    if(*p)
        givenFilter->typeFlags = atoi(p);
    p = p + strlen(p) + 1;
    if(*p)
        givenFilter->rating = atoi(p);
}

Database::Database(const char* filepath)
{
    this->buffer = (char*)malloc(MAXBUFFERSIZE);
    this->dbDescriptor = fopen(filepath, "r");
    memset(this->buffer, 0, MAXBUFFERSIZE * sizeof(char));
    fread(this->buffer, 1, 1024, this->dbDescriptor);
    fclose(this->dbDescriptor);
    this->parsed_json = json_tokener_parse(buffer);
}

bool Database::ConfirmLogon(const char* givenUsername, const char* givenPassword)
{
    json_object* currUser;
    json_object* currUserName;
    json_object* currUserPass;
    for(size_t i = 0, length = json_object_array_length(this->parsed_json); i< length ;i++)
    {
        currUser = json_object_array_get_idx(this->parsed_json, i);
        json_object_object_get_ex(currUser, "username", &currUserName);
        json_object_object_get_ex(currUser, "password", &currUserPass);
        if(!strcmp(givenUsername, json_object_get_string(currUserName)) && 
            !strcmp(givenPassword, json_object_get_string(currUserPass)))
            return true;
    }
    return false;
}



Database::~Database()
{
    parsed_json = nullptr;
    dbDescriptor = nullptr;
}
