[![Build Status](https://travis-ci.org/kamlesh-bambarde/pigeon.svg?branch=master)](https://travis-ci.org/kamlesh-bambarde/pigeon)  [![Gitter chat](https://badges.gitter.im/gitterHQ/services.png)](https://gitter.im/kamlesh-bambarde/pigeon)

##Pigeon

Pigeon is a high performance micro web framework written in C++. It is built on top of libuv which is a multi-platform support library with a focus on asynchronous I/O.

Pigeon is capable of serving static web content and json data via web api (written in C++). It is possible to run Html5 application that's built using AngularJs/ReactJs or any other javascript frameworks, on Pigeon. The web api can be used to serve json data as a response to ajax calls from the Html5 Applications.

##Features

- Http Compression
- Http Pipelining
- Http Filters
- Web Api
- Static Resource Cache
- File Watcher (libuv based)
- multipart/form-data parser
- CORS Headers

##Todo
- SSL Implementation

###Benchmarks
Pigeon performance was compared with that of NodeJS, to serve a static resource of 705 bytes in size, 
weighttp was used to measure the performance of two frameworks and and following is the result.

####Pigeon results
```c++
weighttp -n 100000 -c 64 -t 4 "http://127.0.0.1:8002/index.html"
weighttp 0.4 - a lightweight and simple webserver benchmarking tool

starting benchmark...
<skipped progress notifications>

finished in 2 sec, 386 millisec and 491 microsec, 41902 req/s, 40622 kbyte/s
requests: 100000 total, 100000 started, 100000 done, 100000 succeeded, 0 failed, 0 errored
status codes: 100000 2xx, 0 3xx, 0 4xx, 0 5xx
traffic: 99270840 bytes total, 28770840 bytes http, 70500000 bytes data
```
####NodeJS Results
```c++
weighttp -n 100000 -c 64 -t 4 "http://127.0.0.1:3000/index.html"
weighttp 0.4 - a lightweight and simple webserver benchmarking tool

starting benchmark...
<skipped progress notifications>

finished in 35 sec, 894 millisec and 225 microsec, 2785 req/s, 2701 kbyte/s
requests: 100000 total, 100000 started, 100000 done, 100000 succeeded, 0 failed, 0 errored
status codes: 100000 2xx, 0 3xx, 0 4xx, 0 5xx
traffic: 99300000 bytes total, 28800000 bytes http, 70500000 bytes data
```
As you can see Pigeon has outperformed NodeJS, its time to go native with Pigeon now :).

##Getting Started

[Please refer wiki to get started.](https://github.com/kamlesh-bambarde/pigeon/wiki)

