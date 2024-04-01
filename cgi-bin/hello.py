#!/usr/bin/python3

import cgi, cgitb, os, datetime, sys

cgitb.enable(display=1)
form = cgi.FieldStorage()

def hello(form) -> (int, str) :
	
	# Checks if the file is uploaded
	firstname = form.getvalue('firstname')
	lastname = form.getvalue('lastname')
	if firstname == None or lastname == None:
		return (500, "Please enter a firstname and lastname")
	elif os.environ.get("COOKIE") == None:
		return (500, "Please enable cookies")
	else:
		cookie_string = os.environ.get("COOKIE")

		# Convert the cookie string to a list of items
		cookie_pairs = [item.split("=") for item in cookie_string.split("; ")]

		# Convert the list into a dictionary
		cookie = {key: value for key, value in cookie_pairs}
		return (200, f"Hello {firstname} {lastname}, you have visited our website {cookie['id']} times")

# Get the return values from the function
if os.environ.get("REQUEST_METHOD") == "GET":
	status, message = hello(form)
else:
	status, message = (405, "Method not allowed")

# Get the current date and time in readable format
x = datetime.datetime.now()
date = x.strftime("%a, %d %b %Y %H:%M:%S GMT")

body = f"""<!DOCTYPE html>
<html>
	<head>
		<title>Hello!</title>
	</head>
	<body>
		<h1>{message}</h1>
	</body>
</html>"""

header = f"""HTTP/1.1 {status}\r
Content-Length: {len(body)}\r
Content-type: text/html; charset=utf-8\r
Date: {date}\r
Last-Modified: {date}\r
# Server: {os.environ.get("SERVER")}\r\n\r"""

print(header)
print(body)
print("\0")
