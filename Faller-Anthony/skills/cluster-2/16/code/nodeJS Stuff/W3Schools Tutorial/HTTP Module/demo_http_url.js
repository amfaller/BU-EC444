var http = require('http');

http.createServer(function (req,res) {
	res.writeHead(200, {'Content-Type': 'text/html'});		// 200 means all is OK, second argument is object containing response headers
	res.write(req.url);										// url holds the part of the url that comes after domain name
	res.end();												// end response
}).listen(8080);											// the server object listens on port 8080