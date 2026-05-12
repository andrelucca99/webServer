#!/usr/bin/env python3
import sys
sys.stdout.write("Status: 302 Found\r\n")
sys.stdout.write("Location: /\r\n")
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")
sys.stdout.write("<html><body>redirecting</body></html>")
