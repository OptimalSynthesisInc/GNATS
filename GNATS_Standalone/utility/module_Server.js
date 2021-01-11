
module.exports = {
  startWebServer: function() {
	  var express = require('express');
	  var app = express();
	  var fs = require("fs");
	  var path = require('path');
	  
	  var func_Weather = require('./func_Weather');
	  
	  var NATS_SERVER_HOME = process.env.NATS_SERVER_HOME;
	  //console.log("Environment variable: NATS_SERVER_HOME = " + NATS_SERVER_HOME);
	  
	  var cors = require('cors');
	  app.use(cors());
	  
	  app.use('/web', express.static(__dirname + '/web'));

	  app.get('/', function(req, res) {
		  fs.readFile('web/index.html', function (err, html) {
			    if (err) {
			        throw err;
			    }

		        res.writeHeader(200, {"Content-Type": "text/html"});
		        res.write(html);  
		        res.end();  
		  });
	  });
	  
	  // ============================================================

	  app.get('/createWindFiles', function (req, res) {
		  func_Weather.createWindFiles(req, res);
	  });

	  // ============================================================
	  
	  app.get('/checkStatus_createWindFiles', function (req, res) {
		  func_Weather.checkStatus_createWindFiles(req, res);
	  });

	  // ============================================================
	  
	  // Default IP address: 127.0.0.1 and port number: 3000
	  var server = app.listen(3000, "127.0.0.1", function () {
	      var host = server.address().address
	      var port = server.address().port

	      console.log("GNATS Web Management Server listening at http://%s:%s", host, port)
	  })
  }
};
