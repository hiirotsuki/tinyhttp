# Tinyhttp

Have you ever had the need to share files from one computer to another over a network, but all available options are either broken or overcomplicated for simple file transfer? Look no further than to tinyhttp!

 - can be built without dependency on msvcrt or ucrt (or any other libc)
 - highly portable C89 code, tested with mingw-w64, Pelles C, Visual C++ 4.0
 - only supports HTTP GET requests

## Usage

Simply run the executable and it will serve files from the `www` directory relative to the executable. The server will attempt to create the directory if it does not exist.

Defaults to port 8080. Configurable in tinyhttp.ini
