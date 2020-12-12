var http = require('http');
var url  = require('url');
var fs   = require('fs');

http.createServer(function (req,res) {
	// Parse url, get file name
	var q = url.parse(req.url, true);
	var filename = "." + q.pathname;

	// Opens requested file, returns content to client
	fs.readFile(filename, function(err, data){
		// Throw 404 error if any error
		if(err){
			res.writeHead(404, {'Content-Type': 'text/html'});
      		return res.end("404 Not Found");
		}
		res.writeHead(200, {'Content-Type': 'text/html'});
	    res.write(data);
	    return res.end();
	});
}).listen(8080);