ymaps.ready(function () {
    let myMap = new ymaps.Map("map", { //Cоздание карты
        center: [55.76, 37.64],
        zoom: 10,
        controls: ['geolocationControl', 'typeSelector', 'fullscreenControl', 'zoomControl']
    });

    let geolocation = myMap.controls.get('geolocationControl'); //Кнопка геолокации
    myMap.options.set('dragCursor', 'pointer'); //Установка курсора указателя

    let toLocationInput = document.getElementById('toLocation'); //Поле для локации откуда
    toLocationInput.value = "";
    let fromLocationInput = document.getElementById('fromLocation'); //Поле для локации куда
    fromLocationInput.value = "";

    let disableAutoSwap = false; //Автопереключение откуда-куда

    let toFromChecker = false; //false = from | true = to, переключатель откуда-куда
    document.onclick = function () { //Переключение откуда-куда по клику
        if (toLocationInput == document.activeElement) {
            toFromChecker = toLocationInput == document.activeElement;
        } else if (fromLocationInput == document.activeElement) {
            toFromChecker = fromLocationInput == document.activeElement;
        }
    }

    let searchControl = new ymaps.control.SearchControl({ //Поиск по карте
        options: {
            provider: 'yandex#search',

        }
    });
    let toLocation, fromLocation = []; //Координаты откуда и куда для отправки


    geolocation.events.add('click', function () {//По клику на кнопку геолокации установить позицию как откуда
        ymaps.geolocation.get({provider: 'yandex'}).then(function (result) {
            let resultPosition = result.geoObjects.position;
            [fromLocation, fromLocationInput.value] = [resultPosition,  resultPosition]
        });
    });


    searchControl.events.add('resultselect', function (e) { //При выборе элемента в поиске записать его координаты в соответствии с переключателем откуда-куда
        let coords = searchControl.getResultsArray()[e.get('index')].geometry.getCoordinates();
        if (toFromChecker) {
            [toLocation, toLocationInput.value] = [coords, coords];
        } else {
            [fromLocation, fromLocationInput.value] = [coords, coords];
        }
        if (!disableAutoSwap) toFromChecker = !toFromChecker;

    })


    myMap.controls.add(searchControl); //Отображение поиска

    let placemarkTo, placemarkFrom; //Указатели на откуда и куда на карте
    myMap.events.add('click', function (e) { //Получение координат на карте по клику по ней и установка указателей в соответствии
        let coords = e.get('coords');

        if (toFromChecker) {
            placemarkTo = createToFromPlacemark(coords, placemarkTo);
            [toLocation, toLocationInput.value] = [coords, coords];
        } else {
            placemarkFrom = createToFromPlacemark(coords, placemarkFrom);
            [fromLocation, fromLocationInput.value] = [coords, coords];
        }
        if (!disableAutoSwap) toFromChecker = !toFromChecker;

    });

    let notiClose = document.getElementsByClassName("notiClose")[0], //Для анимации "не может быть построен маршрут"
        noti = document.getElementsByClassName("noti")[0];
    notiClose.onclick = function () {
        noti.classList.add("notiPopout");
        noti.classList.remove("notiPopup")
    }

    let loader = document.getElementById("loader"); //Отображение загрузки

    const connection = new WebSocket("ws://localhost/echo"); //Создание канала


    document.getElementById('button').onclick = function () {

        if (fromLocation != undefined && toLocation != undefined) { //Если ни одна из точек не пустая, то можно отправить

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
                let route;
                for(let i = 0; i < routeCoords["routeCoords"].length; i++){ 
                    if(i < routeCoords["routeCoords"].length-1){ //Составление маршрута с учетом цвета, который зависит от затененности участка
                         route = new ymaps.Polyline([
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

                myMap.geoObjects.add(placemarkTo).add(placemarkFrom); //Отображение маркеров
                placemarkTo = placemarkFrom = null;
                loader.style.display = "none";
            }
            
        } else {
            connection.send(""); //Отправление пингов для поддержания канала
        }


    }


    let clearButton = document.getElementById("clear");
    clearButton.onclick = function () { //Работа кнопки очищения
        toLocationInput.value = fromLocationInput.value = '';
        toLocation = fromLocation = [];
        myMap.geoObjects.remove(placemarkTo);
        myMap.geoObjects.remove(placemarkFrom);
        placemarkTo = placemarkFrom = null;
        toFromChecker = true;
    }

    let clearMapButton = document.getElementById("clearMap"); //Работа кнопки очищения карты
    clearMapButton.onclick = function () {
        myMap.geoObjects.removeAll();
        placemarkTo = placemarkFrom = null;
    }

    let disableButton = document.getElementById("disable"); //Работа кнопки переключения autoswap
    disableButton.onclick = function () {
        disableAutoSwap = !disableAutoSwap;
        this.textContent = disableAutoSwap ? "Enable AutoSwap" : "Disable AutoSwap";
        toFromChecker = !toFromChecker;
    }


    function createPlacemark(coords) { //Функция создания маркера
        return new ymaps.Placemark(coords, {
            iconCaption: 'поиск...'
        }, {
            preset: toFromChecker ? 'islands#redDotIconWithCaption' : 'islands#blueDotIconWithCaption',
            draggable: true
        });
    }

    function getAddress(coords, placemark) { //Запись адреса в маркер
        placemark.properties.set('iconCaption', 'поиск...');
        ymaps.geocode(coords).then(function (res) {
            let firstGeoObject = res.geoObjects.get(0);

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

    function createToFromPlacemark(coords, placemark){ //Создание маркеров начала и конца пути
            if (placemark) {
                placemark.geometry.setCoordinates(coords);
            }
            // Если нет – создаем.
            else {
                placemark = createPlacemark(coords);
                myMap.geoObjects.add(placemark);
                placemark.events.add('dragend', function () {
                    getAddress(placemark.geometry.getCoordinates(), placemark );
                });
            }
            getAddress(coords, placemark);

            return placemark     
    }
});

