ymaps.ready(function () {
    var myMap = new ymaps.Map("map", { //Cоздание карты
        center: [55.76, 37.64],
        zoom: 10,
        controls: ['geolocationControl', 'typeSelector', 'fullscreenControl', 'zoomControl']
    });

    var geolocation = myMap.controls.get('geolocationControl'); //Кнопка геолокации
    myMap.options.set('dragCursor', 'pointer'); //Установка курсора указателя

    var plc, preset; //Точки маршрута и их внешний вид


    var toLocationInput = document.getElementById('toLocation'); //Поле для локации откуда
    toLocationInput.value = "";
    var fromLocationInput = document.getElementById('fromLocation'); //Поле для локации куда
    fromLocationInput.value = "";

    var disableAutoSwap = false; //Автопереключение откуда-куда

    var toFromChecker = true; //true = from | false = to, переключатель откуда-куда
    document.onclick = function () { //Переключение откуда-куда по клику
        if (toLocationInput == document.activeElement) {
            toFromChecker = false;
        } else if (fromLocationInput == document.activeElement) {
            toFromChecker = true;
        }
    }

    var searchControl = new ymaps.control.SearchControl({ //Поиск по карте
        options: {
            provider: 'yandex#search',

        }
    });
    var toLocation, fromLocation = []; //Координаты откуда и куда для отправки


    geolocation.events.add('click', function () {//По клику на кнопку геолокации установить позицию как откуда
        ymaps.geolocation.get({provider: 'yandex'}).then(function (result) {
            fromLocation = result.geoObjects.position;
            fromLocationInput.value = fromLocation;
        });
    });


    searchControl.events.add('resultselect', function (e) { //При выборе элемента в поиске записать его координаты в соответствии с переключателем откуда-куда
        let coords = searchControl.getResultsArray()[e.get('index')].geometry.getCoordinates();
        if (!toFromChecker) {
            toLocation = coords;
            toLocationInput.value = toLocation;
        } else {
            fromLocation = coords;
            fromLocationInput.value = fromLocation;
        }
        if (!disableAutoSwap) toFromChecker = !toFromChecker;

    })


    myMap.controls.add(searchControl); //Отображение поиска

    var placemark1, placemark2; //Указатели на откуда и куда на карте
    myMap.events.add('click', function (e) { //Получение координат на карте по клику по ней и установка указателей в соответсвтии
        var coords = e.get('coords');

        if (!toFromChecker) {
            if (placemark1) {//Проверяем есть ли маркер
                placemark1.geometry.setCoordinates(coords);
            }
            // Если нет – создаем.
            else {
                placemark1 = createPlacemark(coords);
                myMap.geoObjects.add(placemark1);
                placemark1.events.add('dragend', function () {
                    getAddress(placemark1.geometry.getCoordinates(), placemark1);
                });
            }
            getAddress(coords, placemark1);
            toLocation = coords;
            toLocationInput.value = toLocation;
        } else {
            if (placemark2) {
                placemark2.geometry.setCoordinates(coords);
            }
            // Если нет – создаем.
            else {
                placemark2 = createPlacemark(coords);
                myMap.geoObjects.add(placemark2);
                placemark2.events.add('dragend', function () {
                    getAddress(placemark2.geometry.getCoordinates(), placemark2 );
                });
            }
            getAddress(coords, placemark2);
            fromLocation = coords;
            fromLocationInput.value = fromLocation;
        }
        if (!disableAutoSwap) toFromChecker = !toFromChecker;


    });

    var fromToLocations = []; //Объединенный массив откуда, куда для более простой отправки

    var notiClose = document.getElementsByClassName("notiClose")[0], //Для анимации "не может быть построен маршрут"
        noti = document.getElementsByClassName("noti")[0];
    notiClose.onclick = function () {
        noti.classList.add("notiPopout");
        noti.classList.remove("notiPopup")
    }

    var loader = document.getElementById("loader"); //Отображение загрузки

    const connection = new WebSocket("ws://localhost/echo"); //Создание канала


    document.getElementById('button').onclick = function () {

        fromToLocations = [fromLocation, toLocation];
        if (fromToLocations[0] != undefined && fromToLocations[1] != undefined) { //Если ни одна из точек не пустая, то можно отправить

            connection.send(JSON.stringify({
                fromLocation: fromLocation,
                toLocation: toLocation
            }));
            loader.style.display = "block"; //Включение загрузки
        }
    }

    connection.onmessage = (e) => { //При получении сообщения
        if (e.data != "") { //Если сообщение не пинг, для поддержания канала
            routeCoords = JSON.parse(e.data); //Парсим JSON
            if(routeCoords["routeCoords"] == 727){ //Если в routeCoords встречаем код ошибки: невозможно построить маршрут
                
                loader.style.display = "none"; //Отключаем отображение загрузки
                noti.classList.remove("notiPopout"); //Отображаем ошибку
                noti.classList.add("notiPopup")
            }
            else{
                myMap.geoObjects.removeAll(); //Чистим карту от лишнего перед построением
                for(let i = 0; i < routeCoords["routeCoords"].length; i++){ 
                    if(i == 0){ //Управление цветами точек на маршруте
                        preset = 'islands#blueCircleDotIcon';
                    }
                    else if(i == routeCoords["routeCoords"].length-1){
                        preset = 'islands#redCircleDotIcon'
                    }
                    else{
                        preset = 'islands#darkBlueCircleDotIcon'
                    }
                    // !!!ТОЧКИ!!!
                    //plc = createRoutePlacemark(routeCoords["routeCoords"], i, preset);
                    //myMap.geoObjects.add(plc);
                    if(i < routeCoords["routeCoords"].length-1){ //Составление маршрута с учетом цвета, который зависит от затененности участка
                        var route = new ymaps.Polyline([
                            routeCoords["routeCoords"][i],
                            routeCoords["routeCoords"][i+1]
                            ] ,{},
                            {
                                strokeWidth: '15',
                                strokeColor: getColorFromShading(routeCoords["routeShading"][i])
                        });
                    }
                    myMap.geoObjects.add(route);
                }

                /*let route = new ymaps.Polyline(routeCoords["routeCoords"], {},
                {
                    strokeWidth: '15',
                    strokeColor: '#0066ff',
                })*/

                myMap.geoObjects.add(placemark1).add(placemark2); //Отображение маркеров
                placemark1 = placemark2 = null;
                //myMap.geoObjects.add(route);
                loader.style.display = "none";
            }
            /*
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
            */
            
        } else {
            connection.send(""); //Отправление пингов для поддержания канала
        }


    }


    var clearButton = document.getElementById("clear");
    clearButton.onclick = function () { //Работа кнопки очищения
        toLocationInput.value = fromLocationInput.value = '';
        toLocation = fromLocation = [];
        myMap.geoObjects.remove(placemark1);
        myMap.geoObjects.remove(placemark2);
        placemark1 = placemark2 = null;
        toFromChecker = true;
    }

    var clearMapButton = document.getElementById("clearMap"); //Работа кнопки очищения карты
    clearMapButton.onclick = function () {
        myMap.geoObjects.removeAll();
        placemark1 = placemark2 = null;
    }

    var disableButton = document.getElementById("disable"); //Работа кнопки переключения autoswap
    disableButton.onclick = function () {
        disableAutoSwap = !disableAutoSwap;
        this.textContent = disableAutoSwap ? "Enable AutoSwap" : "Disable AutoSwap";
    }


    function createPlacemark(coords) { //Функция создания маркера
        return new ymaps.Placemark(coords, {
            iconCaption: 'поиск...'
        }, {
            preset: toFromChecker ? 'islands#blueDotIconWithCaption' : 'islands#redDotIconWithCaption',
            draggable: true
        });
    }

    function getAddress(coords, placemark) { //Запись адреса в маркер
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


    function createRoutePlacemark(routeCoords, i, preset){ //Создание точек на маршруте
        return new ymaps.Placemark(routeCoords[i], {
        }, {
            preset: preset
        })
    }


    function getColorFromShading(routeShading){ //Конвертация затененности в цвет
        if(routeShading >= 0 && routeShading < .2){
            return "#0066ff"
        }else if(routeShading >= .2 && routeShading < .4){
            return "#5297C6"
        }else if(routeShading >= .4 && routeShading < .6){
            return "#85B6A2"
        }else if(routeShading >= .6 && routeShading < .8){
            return "#BBD67D"
        }else{
            return "#FFFF4D"
        }
    }
});

