web_server
==========
is a header-only web_server that does not yet support HTTPS.

usage
-----
this project uses cmake and asio. Asio will be downloaded on the first build.
You only have to include the "src" folder to use this project, provided you have asio already downloaded.

asio
----
Be sure to set the flag for asio to build as stand-alone.
This can be achieved with the "add_definitions(-DASIO_STANDALONE)" cmake command.

performance_tester
------------------
The performance tester was written to get a rough idea of how fast the web_server is.
It spawns 16 threads which all connect to the root path of the server and get a response.
The time it takes from connection to disconnection is added to a vector

performance
-----------
There is a performance_tester folder which will build a a client to test performance.
I've run the performance_tester on a pc with the following specs:
- AMD Ryzen 9 3900X
- 32 GB RAM

Using 16 threads on the performance_tester I was getting about 18 000 connections with the average response time about 850 microseconds.
This is more than I need so I'm not going to worry about performance too much further.