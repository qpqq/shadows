ymaps.ready(function () {
    var myMap = new ymaps.Map("map", {
      center: [55.76, 37.64],
      zoom: 10,
      controls: ['geolocationControl', 'typeSelector', 'fullscreenControl', 'zoomControl']
    });

    var geolocation = myMap.controls.get('geolocationControl');
    myMap.options.set('dragCursor', 'pointer');


    var toLocationInput = document.getElementById('toLocation');
    toLocationInput.value="";
    var fromLocationInput = document.getElementById('fromLocation');
    fromLocationInput.value="";

    var disableAutoSwap = false;

    var toFromChecker = true; //true = from | false = to
    document.onclick = function(){
    	if(toLocationInput == document.activeElement){
    		toFromChecker = false;
    	}else if(fromLocationInput == document.activeElement){
    		toFromChecker = true;
    	};
    }

    var searchControl = new ymaps.control.SearchControl({
    	options: {
    		provider: 'yandex#search',
            
    	}
    });
    var toLocation, fromLocation = [];


    geolocation.events.add('click', function(){
    	 ymaps.geolocation.get({ provider: 'yandex'}).then(function(result){
	    	fromLocation = result.geoObjects.position;
	    	fromLocationInput.value = fromLocation;
	    });
    });
   


    searchControl.events.add('resultselect', function(e){
    	let coords = searchControl.getResultsArray()[e.get('index')].geometry.getCoordinates();
    	if(!toFromChecker){
    		toLocation = coords;
	    	toLocationInput.value = toLocation;
    	}else{
    		fromLocation = coords;
    		fromLocationInput.value = fromLocation;
    	}
    	if(!disableAutoSwap) toFromChecker = !toFromChecker;
    	
    })


    myMap.controls.add(searchControl);

    var placemark1, placemark2;
	myMap.events.add('click', function (e) {
        var coords = e.get('coords');

        if(!toFromChecker){
        	if (placemark1) {
	            placemark1.geometry.setCoordinates(coords);
	        }
	        // Если нет – создаем.
	        else {
	            placemark1 = createPlacemark(coords);
	            myMap.geoObjects.add(placemark1);
	            placemark1.events.add('dragend', function () {
	                getAddress(placemark1.geometry.getCoordinates());
	            });
	        }
	        getAddress(coords, placemark1);
	        toLocation = coords;
	    	toLocationInput.value = toLocation;
        }else{
        	if (placemark2) {
	            placemark2.geometry.setCoordinates(coords);
	        }
	        // Если нет – создаем.
	        else {
	            placemark2 = createPlacemark(coords);
	            myMap.geoObjects.add(placemark2);
	            placemark2.events.add('dragend', function () {
	                getAddress(placemark2.geometry.getCoordinates());
	            });
	        }
	        getAddress(coords, placemark2);
	        fromLocation = coords;
	    	fromLocationInput.value = fromLocation;
        }
        if(!disableAutoSwap) toFromChecker = !toFromChecker;

        
    });

    var fromToLocations = [];

    var loader = document.getElementById("loader");

    const connection = new WebSocket("ws://localhost/echo");
    

    document.getElementById('button').onclick = function(){
    	
    	fromToLocations = [fromLocation, toLocation];
    	if(fromToLocations[0] != undefined && fromToLocations[1] != undefined){

	    	connection.send(JSON.stringify({
	    		fromLocation: fromLocation,
	    		toLocation: toLocation
	    	}));	
	    	loader.style.display = "block";
    	}
	}

	connection.onmessage = (e) => {
		if(e.data != ""){
			routeCoords = JSON.parse(e.data);
			let route = new ymaps.multiRouter.MultiRoute({
				referencePoints: routeCoords["routeCoords"],
				params: {
					routingMode: "pedestrian"
				}
			}, {
                boundsAutoApply: true,

                 wayPointStartIconFillColor: '#1E98FF',
                 wayPointFinishIconFillColor: 'red',

                routeActivePedestrianSegmentStrokeStyle: "solid",
                routeActivePedestrianSegmentStrokeColor: "#0066ff",
                routeActivePedestrianSegmentStrokeOpacity: '1',
                routePedestrianSegmentStrokeOpacity: "0.5",
                routePedestrianSermentStrokeColor: '#0066ff'
			});

			myMap.geoObjects.removeAll();
			placemark1 = placemark2 = null;
			myMap.geoObjects.add(route);
			loader.style.display = "none";
		}
		else{
			connection.send("");
		}
		

	}


 	var clearButton = document.getElementById("clear");
 	clearButton.onclick = function(){
 		toLocationInput.value = fromLocationInput.value = '';
 		toLocation = fromLocation = []; 
 		myMap.geoObjects.remove(placemark1); 
 		myMap.geoObjects.remove(placemark2); 
 		placemark1 = placemark2 = null;
 		toFromChecker = true;
 	}

 	var clearMapButton = document.getElementById("clearMap");
 	clearMapButton.onclick = function(){
 		myMap.geoObjects.removeAll();
 		placemark1 = placemark2 = null;
 	}

 	var disableButton = document.getElementById("disable");
 	disableButton.onclick = function(){
 		disableAutoSwap = !disableAutoSwap;
 		this.textContent = disableAutoSwap ? "Enable AutoSwap" : "Disable AutoSwap";
 	}

    function createPlacemark(coords) {
        return new ymaps.Placemark(coords, {
            iconCaption: 'поиск...'
        }, {
            preset: toFromChecker ? 'islands#blueDotIconWithCaption' : 'islands#redDotIconWithCaption',
            draggable: true
        });
    }

    function getAddress(coords, placemark) {
        placemark.properties.set('iconCaption', 'поиск...');
        ymaps.geocode(coords).then(function (res) {
            var firstGeoObject = res.geoObjects.get(0);

            placemark.properties
                .set({
                    iconCaption: [
                        firstGeoObject.getLocalities().length ? firstGeoObject.getLocalities() : firstGeoObject.getAdministrativeAreas(),
                        firstGeoObject.getThoroughfare() || firstGeoObject.getPremise()
                    ].filter(Boolean).join(', '),
                    balloonContent: firstGeoObject.getAddressLine()
                });
        });
    }

});

