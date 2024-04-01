#!/usr/bin/env sh

date=`date -u +"%a, %d %b %Y %H:%M:%S GMT"`
message=""
status=""

if [ $REQUEST_METHOD != "GET" ]
then
	message="Method Not Allowed"
	status="405"
else
	message="This script was written in sh"
	status="200"
fi

body="<!DOCTYPE html>
<html>
	<head>
		<title>Bash Test</title>
	</head>
	<body>
		<h1>$message</h1>
	</body>
</html>\r\n\r"

content_length=`echo "$body" | wc -c`

header="HTTP/1.1 200\r
Content-Length: $content_length\r
Content-type: text/html; charset=utf-8\r
Date: $date\r
Last-Modified: $date\r
Server: $SERVER\r\n\r"

echo "$header"
echo "$body"
echo "\0"
