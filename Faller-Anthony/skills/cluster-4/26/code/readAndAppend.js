var fs = require('fs');
const csv = require('csv-parser');

var data=[];		// Var to hold CSV data

// Read CSV file
fs.createReadStream('smoke.csv')
	.pipe(csv())

	.on('data', function(row){
		//console.log(row);
		data.push(row);
	})

	.on('end', function(){
		console.log(data);
	});

//////////////////////////////////////////////////////////////

var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/";

// Insert data to database
MongoClient.connect(url, function(err, db) {
	if (err) throw err;
	var dbo = db.db("skill26db");
	dbo.collection("smokeData").insertMany(data, function(err, res) {
		if (err) throw err;
		console.log("Number of documents inserted: " + res.insertedCount);
		db.close();
	});
});	