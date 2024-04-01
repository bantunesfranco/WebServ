#!/usr/bin/env python3

import datetime, os

x = datetime.datetime.now()
date = x.strftime("%a, %d %b %Y %H:%M:%S GMT")

if os.environ.get("REQUEST_METHOD") == "GET":
	status, message = (200, "This script was written in python")
else:
	status, message = (405, "Method not allowed")

body = f"""<!DOCTYPE html>
<html>
	<head>
		<title>Python Test</title>
	</head>
	<body>
		<h1>{message}</h1>
	</body>
</html>\r\n"""

header = f"""HTTP/1.1 {status}\r
Content-Length: {len(body)}\r
Content-type: text/html; charset=utf-8\r
Date: {date}\r
Last-Modified: {date}\r
Server: {os.environ.get("SERVER")}\r\n\r"""

print(header)
print(body)
print("\0")
