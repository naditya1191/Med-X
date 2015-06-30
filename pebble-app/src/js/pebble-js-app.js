var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
	console.log(this.responseText);
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getData() {
	
	var url = "http://158.130.37.245";
	
	// Send request to OpenWeatherMap
	  xhrRequest(url, 'GET', 
	    function(responseText) {
			
			var str = responseText;
			var data = parseInt(str.substring(23,str.length-9));
			console.log(data);
	      	// responseText contains a JSON object with weather info
		      /*var json = JSON.parse(responseText);

		      // Temperature in Kelvin requires adjustment
		      var counter = json.counter;
		      console.log("Counter is " + responseText);

		      // Conditions
		      //var conditions = json.weather[0].main;      
		      //console.log("Conditions are " + conditions);*/

		      // Assemble dictionary using our keys
		      var dictionary = {
		        0: data
		      };

	      // Send to Pebble
		  Pebble.sendAppMessage(dictionary,
	        function(e) {
	          console.log("Data received!");
	        },
	        function(e) {
	          console.log("Error!");
	        }
	      );
	
	    }      
	  );
	
  	
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getData();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getData();
  }                     
);
