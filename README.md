# Shadows

## Установка

Подробные системные требования фреймворка Drogon:
[ссылка](https://github.com/drogonframework/drogon/wiki/ENG-02-Installation).

Если кратко, то надо установить/обновить эти пакеты

```bash
sudo apt install git
sudo apt install gcc
sudo apt install g++
sudo apt install cmake
sudo apt install libjsoncpp-dev
sudo apt install uuid-dev
sudo apt install openssl
sudo apt install libssl-dev
sudo apt install zlib1g-dev
sudo apt install sqlite3
sudo apt install libsqlite3-dev
```

Устанавливаем сам проект и скачиваем модули

```bash
cd <work_path>
git clone https://github.com/qpqq/shadows.git
cd shadows
git submodule update --init --recursive
```

## Запуск

Перейти в папку `build`, скомпилировать, запустить

```bash
cd build
cmake ..
make
sudo ./shadows
```

Дальше вбить в браузере `localhost` или `http://localhost`, откроется сам сайт

## Использование

Выбираем на карте две точки левой кнопкой мыши, нажимаем `update`, тогда построится маршрут.
Точки можно выбирать встроенным поиском, предварительно нажав на интересующую точку.
Кнопкой `clear` можно отчистить прошлые значения и ввести новые.
