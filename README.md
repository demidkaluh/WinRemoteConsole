# Remote Console

Удаленная консоль для Windows с архитектурой клиент-сервер, аналогичная SSH. 

## Функциональность

### Основные режимы работы

**Сервер (консольный режим):**
```cmd
RemoteConsole.exe -s [port]
```

**Клиент:**
```cmd
RemoteConsole.exe -c [host] [port]
```

**Сервис Windows:**
```cmd
RemoteConsole.exe -service
```

**Управление сервисом:**
```cmd
RemoteConsole.exe -install
RemoteConsole.exe -uninstall
```

### Примеры использования

**Локальный тест:**
```cmd
# Терминал 1 - Сервер
RemoteConsole.exe -s 27015

# Терминал 2 - Клиент  
RemoteConsole.exe -c 127.0.0.1 27015
```

**Удаленное подключение:**
```cmd
# На серверной машине
RemoteConsole.exe -s 27015

# На клиентской машине
RemoteConsole.exe -c 192.168.1.100 27015
```

## Скрипты сборки и управления

### Скрипты в папке `scripts/`

**build.bat** - Сборка проекта
```cmd
scripts\build.bat [debug|release|clean]
```

**run.bat** - Интерактивный запуск с выбором режима
```cmd
scripts\run.bat
```

**test.bat** - Автоматическое тестирование (запускает сервер и клиент)
```cmd
scripts\test.bat
```

**deploy.bat** - Создание пакета для развертывания
```cmd
scripts\deploy.bat
```

+ Реализована фича **C++ wrapper for sockets (4.3)**

## Сборка

```cmd
# Базовая сборка
scripts\build.bat

# Сборка в режиме отладки
scripts\build.bat debug

# Очистка артефактов сборки
scripts\build.bat clean
```

## Структура проекта

```
│   CMakeLists.txt
│   README.md
│
├───scripts
│       build.bat
│       deploy.bat
│       run.bat
│       test.bat
│
└───src
    │   common.h
    │   main.cpp
    │
    ├───client
    │       client.cpp
    │       client.h
    │
    ├───server
    │       server.cpp
    │       server.h
    │
    ├───service
    │       service.cpp
    │       service.h
    │
    └───wrappers
            socket_wrapper.cpp
            socket_wrapper.h
```
