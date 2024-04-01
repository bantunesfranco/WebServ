#!/usr/bin/python3

import pathlib, datetime, os, sys

status = 200
message = ""
value=""
files = []

def is_prefix(path, target_path) -> bool:

	# Check if the path is a prefix of target_path
	target_resolved = pathlib.Path(target_path).resolve()
	path_resolved = pathlib.Path(path).resolve()
	if path_resolved == target_resolved:
		return False
	if path_resolved.parts == target_resolved.parts[:len(path_resolved.parts)]:
		return True
	return False

def listFiles(path) -> (int, str):

	# Check if the path exists
	if path.exists() == False:
		return (404, "Location not found")

	# Check if the path is outside the root directory
	if is_prefix(path, os.environ["PATH_INFO"]) == True:
		return (403, "Forbidden")
	# Loop through the items in the directory
	for f in path.iterdir():
		if f.is_file():
			if "cgi-bin/" in str(f):
				files.append(str(f))
			else:
				files.append(str(f)[len(os.environ.get("PATH_INFO"))-1:])
		elif f.is_dir():
			listFiles(f)

	return (200, "")

def listDirs() -> (int, str):

	root = os.environ.get("PATH_INFO")
	query = os.environ.get("QUERY_STRING")

	# Check if the server sent the root path
	if root != None:

		# Check if the server sent the query string
		# else list the files in the root directory
		global value
		if query != None:
			value = query.split("&")[0]
			if "=" in value:
				value = value.split("=")[1]
			if value == "/cgi-bin/":
				return listFiles(pathlib.Path("./cgi-bin"))
			elif value == "delete":
				return listFiles(pathlib.Path(os.environ.get("UPLOAD_DIR")))
			elif root.split("/")[-1] == value.strip("/"):
				return listFiles(pathlib.Path(root))
			else:
				return listFiles(pathlib.Path(root+"/"+value))
		else:
			status , message = listFiles(pathlib.Path(root))
			if status == 200:
				return listFiles(pathlib.Path("./cgi-bin"))
			else:
				return (status, message)
	else:
		return (500 , "Internal Server Error")

if os.environ.get("REQUEST_METHOD") == "GET":
	status, message = listDirs()
else:
	status, message = (405, "Method not allowed")


x = datetime.datetime.now()
date = x.strftime("%a, %d %b %Y %H:%M:%S GMT")

body_start = """<!DOCTYPE html>
<head>
	<link rel="stylesheet" href="/90s_styles.css">
<head>
<html>
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
		<div>"""
body_end = """
		</div>
	</body>
</html>"""
body_middle = f"		<h1>{message}</h1>\n"

if status == 200:
	body_middle = f"""		<h1>Delete File from upload folder</h2>
		<form onsubmit="deleteFile()" accept-charset="utf-8">\n"""

	for f in files:
		if f.startswith("cgi-bin"):
			p = f"http://{os.environ['HOST']}/{f}"
		elif value == "delete":
			dir = "".join(os.environ.get("UPLOAD_DIR").split("/")[2:])
			p = f"http://{os.environ['HOST']}/{dir}/{f.split('/')[-1]}"
		else:
			p = f"http://{os.environ['HOST']}/{f.split('/')[-1]}"

		body_middle += f"		<input type=\"radio\" id=\"html\" name=\"filename\" value='{f.split('/')[-1]}'>\n<label for=\"css\">{f.split('/')[-1]}</label><br>"
	
	if len(files) == 0:
		body_middle += f"		<h1> Empty Directory </h1>\n"
	else:
		body_middle += """			<br><input type="submit" value="Delete file"/><br><br>
			</form>"""

body = body_start + body_middle + body_end

header = f"""HTTP/1.1 {status}\r
Content-Length: {len(body.encode("utf-8"))}\r
Content-type: text/html; charset=utf-8\r
Date: {date}\r
Last-Modified: {date}\r
Server: {os.environ.get("SERVER")}\r\n\r"""

print(header)
print(body)
print("\0")
