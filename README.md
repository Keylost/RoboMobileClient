# RoboMobileClient

# Описание
Данное программное обеспечение представляет собой клиента для получения телеметрии с робототехнического средства учебно-демонстрационного стенда РТТЗ (роботраффик с техническим зрением)
Серверная часть: https://github.com/Keylost/RoboMobile

# Системные требования
Учебно-демонстрационный стенд РТТЗ (роботраффик с техническим зрением)
Компьютер с ОС Linux/Windows
ПО: Cmake, GCC/Visual Studio, OpenCV,make (только для linux)

# Установка
Для Linux нужно воспользоваться командами
chmod 777 install.sh
./install.sh

Запуск:
cd build
./cl

Для Windows: 
Нужно запустить файл windows_make.bat, после чего перейти в директорию build. Там вы найдете файлы проекта Visual Studio. Откройте cl.sln, выберете конфигурацию Release и откомпилируйте решение.
В папке со сборкой запустите файл cl.exe
