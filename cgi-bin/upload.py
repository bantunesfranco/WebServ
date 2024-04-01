#!/usr/bin/env python3
import datetime, os, cgi, cgitb

cgitb.enable(display=1)
form = cgi.FieldStorage()

def genFilename(filePath) -> str:

	#split the file name and extension
	fileName, fileExtension = os.path.splitext(filePath)

	#add a number to the filename
	index = 1
	filePath = fileName + str(index) + fileExtension

	# Generate a new filename if the file already exists
	while os.path.exists(filePath) ==  True:
		index += 1
		filePath = fileName + str(index) + fileExtension

	return filePath


def uploadFile(form) -> (int, str) :

	# Create uploads folder if it doesn't exist
	uploadDir = os.environ.get("UPLOAD_DIR")
	if os.path.exists(uploadDir) == False:
		os.mkdir(uploadDir)

	if os.path.exists(os.environ.get("UPLOAD_DIR")) == False:
		return (500, "Internal Server Error")
	
	# Checks if the file is uploaded
	if form.getvalue('filename'):
		fileitem = form['filename']

		# Test if the file was previous
		fn = os.path.basename(fileitem.filename.replace("\\", "/"))
		filePath = os.path.join(uploadDir, fn)

		if os.path.exists(filePath) == True:
			filePath = genFilename(filePath)

		# Write the file to the uploads folder
		try:
			with open(os.path.join(filePath), 'wb') as f:
				f.write(fileitem.file.read())
			return (201, f"\"{fn}\" uploaded successfully!!!")
		except:
			return (500, "Upload failed!!!")
	else:
		return (200, "No file was uploaded!!!")

# Get the return values from the function
if os.environ.get("REQUEST_METHOD") == "POST":
	status, message =  uploadFile(form)
else:
	status, message = (405, "Method not allowed")

# Get the current date and time in readable format
x = datetime.datetime.now()
date = x.strftime("%a, %d %b %Y %H:%M:%S GMT")

body = f"""<!DOCTYPE html>
<html>
	<head>
		<title>File Upload</title>
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
			<h1>{message}</h1>
		</div>
	</body>
</html>"""

header = f"""HTTP/1.1 {status}\r
Content-Length: {len(body)}\r
Content-type: text/html; charset=utf-8\r
Connection: close\r
Date: {date}\r
Last-Modified: {date}\r
Server: {os.environ.get("SERVER")}\r\n\r"""

print(header)
print(body)
print("\0")
