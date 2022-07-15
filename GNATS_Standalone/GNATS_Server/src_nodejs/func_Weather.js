var stdout_data_createWindFiles = "";

var NATS_SERVER_HOME = process.env.NATS_SERVER_HOME;

var moment = require("moment");

var fs = require("fs");

var logger;

module.exports = {
	createWindFiles: function(req, res) {
		  const readline = require('readline');

		  var exec = require('child_process').exec;
		  
		  var exec1 = require('child_process').exec;
		  var exec2 = require('child_process').exec;

		  var tmpStr;

		  logger = fs.createWriteStream(NATS_SERVER_HOME + '/log/WebServer_' + moment().format('YYYYMMDD_HHmmss') + '.log', {
			  flags: 'a' // 'a' means appending
			  });
		  
		  // Delete rap.new directory
		  tmpStr = "Deleting directory: " + NATS_SERVER_HOME + "/share/tg/rap.new";
		  logger.write(tmpStr+"\n");
		  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + tmpStr;
		  var p1 = exec1("rm -rf " + NATS_SERVER_HOME + "/share/tg/rap.new");
		  p1.stdout.on('data', function(data) {
			  //console.log(data);
			  logger.write(data);
				if ((data != "") && (data != "\n")) {
					  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + data;
				}
			  });

		  p1.stderr.on('data', function(data) {
			  //console.log(data);
			  logger.write(data);
				if ((data != "") && (data != "\n")) {
					  stdout_data_createWindFiles = stdout_data_createWindFiles + "\nError: " + data;
				}
			  });

		  // Delete rap.bak directory
		  tmpStr = "Deleting directory: " + NATS_SERVER_HOME + "/share/tg/rap.bak";
		  logger.write(tmpStr+"\n");
		  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + tmpStr;
		  var p2 = exec2("rm -rf " + NATS_SERVER_HOME + "/share/tg/rap.bak");
		  p2.stdout.on('data', function(data) {
			  //console.log(data);
			  logger.write(data);
				if ((data != "") && (data != "\n")) {
					  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + data;
				}
			  });
		  
		  p2.stderr.on('data', function(data) {
			  //console.log(data);
			  logger.write(data);
  				if ((data != "") && (data != "\n")) {
  					  stdout_data_createWindFiles = stdout_data_createWindFiles + "\nError: " + data;
  				}
  			  });
		  
		  // ********************************************************

		  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n";
		  logger.write("\n");
		  
		  var execProcess = exec('python -u createWindFiles.py ' + NATS_SERVER_HOME + '/share/tg/rap.new',
				  {cwd: NATS_SERVER_HOME + '/utility'});
		  execProcess.stdout.on('data', function(data) {
			  //console.log(data);
			  logger.write(data);
			  if ((data != "") && (data != "\n")) {
				  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + data;
			  }
		  });

		  execProcess.stderr.on('data', function(data) {
			  //console.log(data);
			  logger.write(data);
				if ((data != "") && (data != "\n")) {
					  stdout_data_createWindFiles = stdout_data_createWindFiles + "\nError: " + data;
				}
			  });

		  // ********************************************************
		  
		  res.send({data: 'createWindFiles processing'});
	},

	// ============================================================

	checkStatus_createWindFiles: function(req, res) {
		  if (!stdout_data_createWindFiles.includes("Good bye.")) {
			  res.send({data: stdout_data_createWindFiles});
		  } else {
			  var tmpStr = stdout_data_createWindFiles;
			  var exec = require('child_process').exec;
			  
			  var flag_rap_dir_exist = false;
			  
			  fs.stat(NATS_SERVER_HOME + "/share/tg/rap", function (err, stats) {
				  if (err) {
					  // Do nothing
				  } else {
					  flag_rap_dir_exist = true;
				  }
			  });
			  
			  if (flag_rap_dir_exist) {
				  // Rename folder rap to rap.bak
				  tmpStr = tmpStr + "\nRenaming directory: " + NATS_SERVER_HOME + "/share/tg/rap to " + NATS_SERVER_HOME + "/share/tg/rap.bak";
				  logger.write(tmpStr+"\n");
				  
				  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + tmpStr;

				  exec("mv " + NATS_SERVER_HOME + "/share/tg/rap " + NATS_SERVER_HOME + "/share/tg/rap.bak");
			  }
			  
			  // Rename folder rap.new to rap
			  tmpStr = tmpStr + "\nRenaming directory: " + NATS_SERVER_HOME + "/share/tg/rap.new to " + NATS_SERVER_HOME + "/share/tg/rap";
			  logger.write(tmpStr+"\n");
			  
			  stdout_data_createWindFiles = stdout_data_createWindFiles + "\n" + tmpStr;
			  
			  exec("mv " + NATS_SERVER_HOME + "/share/tg/rap.new " + NATS_SERVER_HOME + "/share/tg/rap");
			  
			  exec("rm -f " + NATS_SERVER_HOME + "/share/tg/rap/*.grib2");
			  
			  tmpStr = tmpStr + "\nDeleting directory: " + NATS_SERVER_HOME + "/share/tg/rap.bak";
			  exec("rm -rf " + NATS_SERVER_HOME + "/share/tg/rap.bak");

			  tmpStr = tmpStr + "\nCreateWindFiles Finished.";
			  
			  logger.write(tmpStr);
			  
			  // Send HTML response back
			  res.send({data: tmpStr});
		  }
		  
		  stdout_data_createWindFiles = ""; // Reset
	}
}