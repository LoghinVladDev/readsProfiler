Project template : ReadsProfiler. Nu e gata, obviously.
Ca sa compilezi : 
    server : g++ serverReadsProfiler.cpp -o <exec name, i.e. Server> -lpthread -ljson-c
    client : g++ clientReadsProfiler.cpp -o <exec name, i.e. Client>
Ca sa rulezi fara (segmentation fault)
    -trebuie sa existe in dir-u executabilei un director numit "logs", care sa contina doua subdirectoare
        numite "client" si "server"
    -trebuie sa existe un accounts.json echivalent in format cu cel din arhiva
    -dupa toate, executi ./Server intai, il lasi asa, si din alt(e) terminal(e), executi ./Client,
        folosind user & pass existent in json
Build shortcut :
g++ serverReadsProfiler.cpp -o Server -lpthread -ljson-c &&
g++ clientReadsProfiler.cpp -o Client