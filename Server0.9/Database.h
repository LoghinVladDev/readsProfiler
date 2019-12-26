#pragma once
#include <json-c/json.h>
#include <stdio.h>
#include <cstring>
#define MAXBUFFERSIZE 65532
#define _DB_DRAMA 1
#define _DB_COMED 2
#define _DB_THRIL 4
#define _DB_HORRO 8
#define _DB_MANUA 16
#define _DB_BIOGR 32
#define _DB_ABIOG 64
#define _DB_CHILD 128
#define _DB_ACTIO 256
#define _DB_FANTA 512
#define _DB_ROMAN 1024
#define _DB_ALLGN 2047

#define _isInGenreFlag(givenGenre) \
(\
    if(!strcmp(givenGenre, "drama"))\
         _DB_DRAMA;\
    if(!strcmp(givenGenre, "comedy"))\
         _DB_COMED;\
    if(!strcmp(givenGenre, "thriller"))\
         _DB_THRIL;\
    if(!strcmp(givenGenre, "horror"))\
         _DB_HORRO;\
    if(!strcmp(givenGenre, "manual"))\
         _DB_MANUA;\
    if(!strcmp(givenGenre, "biography"))\
         _DB_BIOGR;\
    if(!strcmp(givenGenre, "autobiography"))\
         _DB_ABIOG;\
    if(!strcmp(givenGenre, "children"))\
         _DB_CHILD;\
    if(!strcmp(givenGenre, "action"))\
        _DB_ACTIO;\
)


/**
 * External function used to convert from string to flag
 * 
 * Returns the flag of the given gerne string
 */
int isInGenreFlag(const char* givenGenre)
{
    if(!strcmp(givenGenre, "drama"))
        return _DB_DRAMA;
    if(!strcmp(givenGenre, "comedy"))
        return _DB_COMED;
    if(!strcmp(givenGenre, "thriller"))
        return _DB_THRIL;
    if(!strcmp(givenGenre, "horror"))
        return _DB_HORRO;
    if(!strcmp(givenGenre, "manual"))
        return _DB_MANUA;
    if(!strcmp(givenGenre, "biography"))
        return _DB_BIOGR;
    if(!strcmp(givenGenre, "autobiography"))
        return _DB_ABIOG;
    if(!strcmp(givenGenre, "children"))
        return _DB_CHILD;
    if(!strcmp(givenGenre, "action"))
        return _DB_ACTIO;
    if(!strcmp(givenGenre, "romance"))
        return _DB_ROMAN;
    if(!strcmp(givenGenre, "fantasy"))
        return _DB_FANTA;
}

/**
 * Structure describing a search filter, used for database querry
 * 
 * (int) page = which page does the user request?
 * (int) entriesPerPage = How many entries does the user Request?
 * (char*) bookName = What title does the user look for?
 * (char*) authorName = Which author's work does the user look for?
 * (int) minYear = publications after [minYear] age
 * (int) maxYear = publications before [maxYear] age
 * (int) typeFlags = which genres does the user look for?
 *      genres are stored in a flag which is in base 2, i.e.
 *      - user is looking for both thriller and horror books, thriller flag is 4 (100), horror flag is 8 (1000), therefore
 *      - what he is looking for can be described as 12 (1100)
 * (int) rating = above what rating does the user want entries to be?
 */ 
struct DBFilter{
    //which page does the user request?
    int page;
    //How many entries does the user Request?
    int entriesPerPage;
    //What title does the user look for?
    char *bookName;
    //authorName = Which author's work does the user look for?
    char *authorName;
    //publications after [minYear] age
    int minYear;
    //publications before [maxYear] age
    int maxYear;
    /**which genres does the user look for?
     *      genres are stored in a flag which is in base 2, i.e.
     *      - user is looking for both thriller and horror books, thriller flag is 4 (100), horror flag is 8 (1000), therefore
     *      - what he is looking for can be described as 12 (1100)
     */
    int typeFlags;
    //above what rating does the user want entries to be?
    int rating;
};


/**
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

//Header of the Database class
class Database
{
    private:
        char* linkedDatabaseRelativePath;
        /**
         * Address to the database file the current object is linked to.
        */
        FILE* dbDescriptor{nullptr};

        /**
         * Parsed database string, type json_object, converted from buffer
         */
        json_object *parsed_json{nullptr};

        /**
         * Buffer used throughout the class functions, mostly to read from database file.
         */
        char* buffer{nullptr};
    public:

        /**
          * Constructor of the Database object.
          *    
          * First argument must contain the relative/absolute path of the database file [.json]
          * Second argument is optional, defaulted to MAXBUFFERSIZE = 8 bytes. Contains database file max size, change for larger databases.
        */
        Database(const char*, const int& = MAXBUFFERSIZE);

        /**
         * Function used to decode a filter from char* (client filter format request) to DBFilter*, a structure containing filter variables
         * 
         * First argument is the address of the filter in which the settings will be saved
         * Second argument is the given string containing coded filter
         */
        void DecodeFilter(DBFilter *, const char*);

        /**
         * Function used to request a query from the database
         * 
         * First argument is the address of the result array (DBResult*) in which the results will be saved
         * Second argument is the max size of the result array
         * Third argument is an address to the filter structure given to create the query. Required.
         * 
         * Return is number of entries saved in the result array
         */
        int HandleQuery(DBResult*, const int&, const DBFilter*);

        /**
         * Function that looks into the users database and compares a client's login information to existing ones.
         * 
         * First argument is the string in which the given username is stored
         * Second argument is the string in which the given password is stored
         * 
         * returns true on login success (user and pass exist in the database), false otherwise
         */
        bool ConfirmLogon(const char*, const char *);

        void AddEntry(const char*, const char*);

        bool existsAccount(const char*);

        /**
         * Database object destructor
         */
        ~Database();
};

int Database::HandleQuery(DBResult* givenArray, const int& arrayLen,const DBFilter* givenFilter)
{
    size_t index = 0;
    json_object *bookInfo, *bookTitle,  *bookAuthor, *bookYear, *bookGenre, *bookRating,
        *bookISBN, *bookDiskpath;
    
    for(size_t i = 0, length = json_object_array_length(this->parsed_json); i< length; i++)
    {
        bookInfo = json_object_array_get_idx(this->parsed_json, i);
        json_object_object_get_ex(bookInfo, "title", &bookTitle);
        json_object_object_get_ex(bookInfo, "author", &bookAuthor);
        json_object_object_get_ex(bookInfo, "year", &bookYear);
        json_object_object_get_ex(bookInfo, "genre", &bookGenre);
        json_object_object_get_ex(bookInfo, "rating", &bookRating);
        json_object_object_get_ex(bookInfo, "isbn", &bookISBN);
        json_object_object_get_ex(bookInfo, "diskPath", &bookDiskpath);
        if(givenFilter->minYear <= json_object_get_int(bookYear) && json_object_get_int(bookYear) <= givenFilter->maxYear)
        {
            if(givenFilter->rating <= json_object_get_int(bookRating))
            {
                if(isInGenreFlag(json_object_get_string(bookGenre)) & givenFilter->typeFlags)
                {
                    if(strstr(json_object_get_string(bookAuthor),givenFilter->authorName))
                    {
                        if(strstr(json_object_get_string(bookTitle), givenFilter->bookName))
                        {
                            if((givenFilter->page-1)*givenFilter->entriesPerPage <= index && 
                                index < givenFilter->page * givenFilter->entriesPerPage)
                            {
                                memset(givenArray[index].authorName, 0, sizeof(DBResult::authorName));
                                memset(givenArray[index].bookName, 0, sizeof(DBResult::bookName));
                                memset(givenArray[index].diskPath, 0, sizeof(DBResult::diskPath));
                                memset(givenArray[index].genre, 0, sizeof(DBResult::genre));
                                strcpy(givenArray[index].authorName, json_object_get_string(bookAuthor));
                                strcpy(givenArray[index].bookName, json_object_get_string(bookTitle));
                                strcpy(givenArray[index].genre, json_object_get_string(bookGenre));
                                strcpy(givenArray[index].diskPath, json_object_get_string(bookDiskpath));
                                givenArray[index].ISBN = json_object_get_int(bookISBN);
                                givenArray[index].rating = json_object_get_int(bookRating);
                                givenArray[index].year = json_object_get_int(bookYear);
                                index++;
                            }
                        }
                    }
                }
            }
        }
    }
    return index;
}

void Database::AddEntry(const char* username, const char* password)
{
    char* databaseBuffer = (char*)calloc(sizeof(char),MAXBUFFERSIZE);
    this->dbDescriptor = fopen(this->linkedDatabaseRelativePath, "r");
    fread(databaseBuffer, 1, MAXBUFFERSIZE, this->dbDescriptor);
    fclose(this->dbDescriptor);
    char* pointer;

    pointer = databaseBuffer;

    while(strchr(pointer, '}')) pointer = strchr(pointer, '}') + 1;

    sprintf(pointer, ",\n\t{\n\t\t\"username\" : \"%s\",\n\t\t\"password\" : \"%s\"\n\t}\n]", username, password);

    this->dbDescriptor = fopen(this->linkedDatabaseRelativePath, "w");

    fputs(databaseBuffer, this->dbDescriptor);

    fclose(this->dbDescriptor);

    free(databaseBuffer);
}

bool Database::existsAccount(const char* username)
{
    json_object *entry;
    json_object *entryName;
    for (size_t i = 0, length = json_object_array_length(this->parsed_json); i<length; i++)
    {
        entry = json_object_array_get_idx(this->parsed_json, i);
        json_object_object_get_ex(entry, "username", &entryName);
        if(!strcmp(username, json_object_get_string(entryName)))
            return true;
    }
    return false;
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
        printf("%s;%s;\n%s;%s;\n", givenUsername, givenPassword, json_object_get_string(currUserName), json_object_get_string(currUserPass));
        if(!strcmp(givenUsername, json_object_get_string(currUserName)) && 
            !strcmp(givenPassword, json_object_get_string(currUserPass)))
            return true;
    }
    return false;
}


void Database::DecodeFilter(DBFilter* givenFilter, const char* givenInfo)
{
    char* infoCopy = (char*)malloc(1024), *p;
    perror("1");
    memset(givenFilter->authorName,0 ,128);
    perror("2");
    memset(givenFilter->bookName, 0 ,128);
    perror("3");
    givenFilter->entriesPerPage = 20;
    givenFilter->page = 1;
    givenFilter->minYear = -1;
    givenFilter->maxYear = 999999;
    givenFilter->rating = -1;
    givenFilter->typeFlags = _DB_ALLGN;
    perror("4");
    memset(infoCopy, 0, 1024);
    perror("5");
    strcpy(infoCopy, givenInfo);
    perror("6");
    p = infoCopy;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        givenFilter->page = atoi(p);
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        givenFilter->entriesPerPage = atoi(p);
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        strcpy(givenFilter->authorName, p);   
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        strcpy(givenFilter->bookName, p);  
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        givenFilter->minYear = atoi(p); 
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        givenFilter->maxYear = atoi(p); 
    p = p + strlen(p) + 1;
    *strchr(p, ';') = 0;
    printf("%s\n", p);
    if(*p)
        givenFilter->typeFlags = atoi(p);
    p = p + strlen(p) + 1;
    printf("%s\n", p);
    if(*p)
        givenFilter->rating = atoi(p);
    perror("b");
}

Database::Database(const char* filepath,const int& bufferLength)
{
    this->linkedDatabaseRelativePath = (char*)malloc(256);
    strcpy(this->linkedDatabaseRelativePath, filepath);
    this->buffer = (char*)malloc(bufferLength);
    this->dbDescriptor = fopen(this->linkedDatabaseRelativePath, "r");
    memset(this->buffer, 0, bufferLength * sizeof(char));
    fread(this->buffer, 1, bufferLength, this->dbDescriptor);
    fclose(this->dbDescriptor);
    this->parsed_json = json_tokener_parse(buffer);
}

Database::~Database()
{
    parsed_json = nullptr;
    dbDescriptor = nullptr;
}
