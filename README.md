BaoIP
=====

A minimalist IP stack written in ANSI C. 

It trades memory for CPU time, which generally fits in with what most people want.

It uses raw sockets under Linux. I started out with a PCAP version but the latency was pretty severe (3ms - 5ms instead of 250us - 300us).

It is built as a shared library with a simple API to allow:

- Initializing the stack
- Pushing data into the stack
- Providing a function for the stack to send data

This will likely expand, although part of the reason it is being done as an isolated library is to ultimately allow for it to be ported to other operating systems such as [BareMetal OS](https://github.com/ReturnInfinity/BareMetal-OS).

Another key reason for this project's existence is to help improve my low-level networking knowledge and to improve my C skills. You're likely to find bugs, strange implementations and all manner of weird and not to wonderful things in here. I'm always up for feedback, but remember, I don't claim this is a professional project, just something small to pass the time :)


Credits
-------
BaoIP is heavily inspired by the work of [Ian Seyler](https://github.com/IanSeyler) [@ReturnInfinity](http://www.returninfinity.com/), in particular [BareMetal OS](https://github.com/ReturnInfinity/BareMetal-OS) and more recently [MinIP](https://github.com/IanSeyler/minIP) - indeed BaoIP "borrows" (with permission) some code from the latter project. 

Building
--------

You'll need at least CMake version 2.8 to build this project. To build:

    $ git clone https://github.com/pmembrey/BaoIP.git
    $ cd BaoIP
    $ mkdir build
    $ cd build
    $ cmake -G "Unix Makesfiles" ../
    $ make

Usage
-----
To run the app:

    $ ./stack <eth device> <ip address> <netmask>

If you receive a permissions error, you can use _setcap_ to grant them (as root):

    # setcap cap_net_raw+ep ./stack 

TODO:
-----
- Optimize alignment of various structures
- Sort out proper namespacing so that BaoIP can play nice with others
- Change the ping code so it uses the length in the IP header rather than making assumptions about the packet length
- Remove as many instances of "strncmp" as possible - go for native integer compares