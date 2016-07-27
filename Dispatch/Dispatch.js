var config = require('./config');

var RedisSMQ = require("rsmq");
var rsmq = new RedisSMQ({
	host: config.redis.host,
	port: config.redis.port,
	ns: "rsmq"
});
var AWS = require('aws-sdk');
var s3 = new AWS.S3();
exports.handler = function(event, context, callback) {
	var params = {};

	if (event.Records.length > 0 && event.Records[0].s3) {
		params = {
			Bucket: event.Records[0].s3.bucket.name,
			Key: event.Records[0].s3.object.key
		};
		s3.getObject(params, function(err, data) {
			if (err) callback(err, null); // an error occurred
			else {
				rsmq.sendMessage({
					qname: config.redis.usersQueue,
					message: data.Metadata.message
				}, function(err, resp) {
					if (resp) {
						context.done(null, 'Function finished!'); 
					} else {
						//if we get the error here lambda will retry next three times
						callback(err, null);
					}
				});
			}
		});
	} else {
		context.done(null, 'Event error');
	}

};