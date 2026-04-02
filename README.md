# C-http-server

This was an exercise in Unix network programming. This is a http-server that hosts itself on your local machine and is accessible on your local network (Or any machine trying to connect to it however you please).

## How it works

It does the basic

```
socket
  |
  v
bind
  |
  v
listen
  |
  v
accept
```

Loop which other http servers do as well.

Then the threads take over.

There is one main discovery thread which checks all of the incoming connnections and then dispatches a connection to the other threads. Then a worker thread picks it up, servers the requested information, then closes the connection.

Initially there are 10 worker threads, each of them waiting for incoming connections and data going out. This can be changed via `init_threads(int, int)` By modifying the number of threads.
