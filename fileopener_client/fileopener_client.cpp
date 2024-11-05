#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFileInfo>
#include <QDebug>

int main(int argc, char *argv[]) {
    // Создание объекта приложения Qt без графического интерфейса
    QCoreApplication app(argc, argv);

    // Проверка количества аргументов командной строки
    if (argc != 2) {
        // Вывод критического сообщения о неправильном использовании программы
        qCritical() << "Usage:" << argv[0] << "<file_path>";
        return 1; // Завершение программы с кодом ошибки 1
    }

    // Преобразование переданного пути к файлу из локальной кодировки в QString
    QString filePath = QString::fromLocal8Bit(argv[1]);
    // Создание объекта QFileInfo для проверки существования файла и получения информации о нем
    QFileInfo fi(filePath);

    // Проверка существования файла
    if (!fi.exists()) {
        // Вывод критического сообщения, если файл не существует
        qCritical() << "File does not exist:" << filePath;
        return 1; // Завершение программы с кодом ошибки 1
    }

    // Получение расширения файла и преобразование его в нижний регистр
    QString extension = fi.suffix().toLower();
    // Вывод информации о расширении файла для отладки
    qDebug() << "File extension:" << extension;

    // Создание интерфейса D-Bus для взаимодействия с сервисом
    QDBusInterface iface(
        "com.example.FileOpenerService", // Имя D-Bus сервиса
        "/FileOpener",                    // Путь к объекту на D-Bus
        "com.example.FileOpener",         // Имя интерфейса D-Bus
        QDBusConnection::sessionBus()     // Использование сессионного D-Bus
    );

    // Проверка валидности интерфейса D-Bus
    if (!iface.isValid()) {
        // Вывод критического сообщения, если не удалось подключиться к сервису
        qCritical() << "Failed to connect to D-Bus service:" 
                    << QDBusConnection::sessionBus().lastError().message();
        return 1; // Завершение программы с кодом ошибки 1
    }

    // Вызов метода OpenFile на D-Bus сервисе с передачей пути к файлу
    QDBusReply<bool> reply = iface.call("OpenFile", filePath);

    // Проверка валидности ответа от D-Bus
    if (reply.isValid()) {
        if (reply.value()) {
            // Вывод отладочного сообщения при успешном открытии файла
            qDebug() << "File opened successfully.";
        } else {
            // Вывод предупреждающего сообщения, если открытие файла не удалось
            qWarning() << "Failed to open file.";
        }
    } else {
        // Вывод критического сообщения, если вызов D-Bus метода завершился ошибкой
        qCritical() << "D-Bus call failed:" << reply.error().message();
        return 1; // Завершение программы с кодом ошибки 1
    }

    return 0; // Успешное завершение программы
}
