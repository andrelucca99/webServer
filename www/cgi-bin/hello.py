#!/usr/bin/env python3
import os
import sys

method = os.environ.get("REQUEST_METHOD", "")
query  = os.environ.get("QUERY_STRING", "")
ctype  = os.environ.get("CONTENT_TYPE", "")
clen   = os.environ.get("CONTENT_LENGTH", "")

body = ""
if method == "POST":
    try:
        n = int(clen) if clen else 0
    except ValueError:
        n = 0
    if n > 0:
        body = sys.stdin.read(n)

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("X-CGI-Echo: hello\r\n")
sys.stdout.write("\r\n")
sys.stdout.write("<html><body>")
sys.stdout.write("<h1>hello-cgi</h1>")
sys.stdout.write("<p>method=" + method + "</p>")
sys.stdout.write("<p>query=" + query + "</p>")
sys.stdout.write("<p>content_type=" + ctype + "</p>")
sys.stdout.write("<p>body=" + body + "</p>")
sys.stdout.write("</body></html>")
