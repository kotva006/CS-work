A Simple Web Server
===================

This is a program that runs a simple web server that will process GET
requests.

In order to run the code it first needs to run make.  Then the resulting
executable is of the following form:

```
./web_server_http port_num path_to_root num_dispatchers num_workers queue_length cache_length
```

*port_num is the port you wish to run the web server through
*path_to_root is the path to the root of where the hosted files are located
*num_dispatchers is the number of threads that will listen for requests
*num_workers is the number of threads that will be fetching files
*queue_length is the lenght of queue that will hold the requests that have yet to be processed
*cache_length is how many objects will be stored in cache
