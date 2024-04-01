#!/usr/bin/env sh

status=""
message=""
filename=""
dir=""

# checks if request method is DELETE
if [ "$REQUEST_METHOD" != "DELETE" ]
then
	message="Method Not Allowed"
	status="405"
fi

# checks if the query string is empty and if no argument was given
if [ -z "$status" ] && [ -z "$QUERY_STRING" ]  && [ -z "$1" ]
then
	message="No file specified"
	status="400"
else
	# checks if an argument was given
	if [ -z "$1" ]
	then
		# gets the filename from the query string
		filename=`echo "$QUERY_STRING" | awk -F'=' '{print $2}' | cut -d'&' -f1 | sed 's/%20/ /g'`
	else
		# gets the filename from the argument depending on the format
		if [ -z `echo "$1" | grep "="` ]
		then
			filename="$1"
		else
			filename=`echo "$1" | awk -F'=' '{print $2}' | sed 's/%20/ /g'`
		fi
	fi
fi

# checks if the server sent the upload directory
if [ -z "$status" ] && [ -z "$UPLOAD_DIR" ]
then
	message="Internal Server Error"
	status="500"
fi

# checks if status was previously set
if [ -z "$status" ]
then
	# checks if the file exists
	if [ -f "$UPLOAD_DIR/$filename" ]
	then
		rm -rf "$UPLOAD_DIR/$filename" 2> /dev/null > /dev/null 

		# checks if the file was deleted successfully
		if [ $? -eq 0 ]
		then
			message="File deleted successfully"
			status="200"
		else
			message="File could not be deleted"
			status="500"
		fi
	else
		message="File does not exist"
		status="200"
	fi
fi

# sets the date
date=`date -u +"%a, %d %b %Y %H:%M:%S GMT"`

body="<!DOCTYPE html>
<html>
	<head>
		<title>Delete File</title>
		<link rel="stylesheet" href="/90s_styles.css">
	</head>
	<body>
		<nav class="left-menu">
			<ul>
			<li><a href="/index.html"><img class="small" src="/cookie.png"></a></li>
			<li><a href="/upload.html">Upload</a></li>
			<li><a href="/delete.html">Delete</a></li>
			<li><a href="/other.html">Cookies</a></li>
			<li><a href="/sockets.html">Sockets</a></li>
			<li><a href="/epoll.html">Epoll</a></li>
			</ul>
		</nav>
		<div class="container">
			<h1>$message</h1>
		</div>
	</body>
</html>\r\n"

# gets the length of the body
content_length=`echo "$body" | wc -c`

header="HTTP/1.1 $status
Content-Length: $content_length
Content-type: text/html; charset=utf-8
Date: $date
Last-Modified: $date
Connection: close
Server: $SERVER"

echo "$header"
echo ""
echo "$body"
echo "\0"

