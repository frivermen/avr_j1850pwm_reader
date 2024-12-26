Проект для чтения шины SAE J1850 PWM (41.6Kb/s) на моём Ford Focus mk1 2001 года выпуска.

Из-за низкой скорости atmega328p 16MHz, мне не удалось реализовать полный алгоритм распознавания посылок,
поэтому сделал упрощённую - определение SOF(Start Of Frame) по +5V импульсу (32мкс), и чтение битов, 
конец по таймауту от спадающего фронта(40мкс).

Можно легко добавить проверку контрольной суммы, но для моей задачи показывать скорость и температуру двигателя
она не нужна.

Есть пример подключения в папке dashboard_example.
