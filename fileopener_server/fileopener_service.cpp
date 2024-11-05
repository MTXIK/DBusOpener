#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QProcess>
#include <QObject>
#include <QFileInfo>
#include <QDebug>

// Класс сервиса с явным указанием D-Bus интерфейса
class FileOpener : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.example.FileOpener") // Указание имени D-Bus интерфейса

public:
    // Конструктор класса FileOpener
    FileOpener() {
        // Подключение к сессионному D-Bus (для текущего пользователя)
        QDBusConnection connection = QDBusConnection::sessionBus();
        if (!connection.isConnected()) { // Проверка успешного подключения
            qCritical() << "Cannot connect to the D-Bus session bus:" << connection.lastError().message();
            QCoreApplication::exit(1); // Завершение приложения с кодом ошибки
        }

        // Регистрация D-Bus сервиса с именем "com.example.FileOpenerService"
        if (!connection.registerService("com.example.FileOpenerService")) {
            qCritical() << "Failed to register D-Bus service:" << connection.lastError().message();
            QCoreApplication::exit(1); // Завершение приложения с кодом ошибки
        }

        // Регистрация объекта D-Bus по пути "/FileOpener" и экспорт всех слотов как D-Bus методов
        if (!connection.registerObject("/FileOpener", this, QDBusConnection::ExportAllSlots)) {
            qCritical() << "Failed to register D-Bus object:" << connection.lastError().message();
            QCoreApplication::exit(1); // Завершение приложения с кодом ошибки
        }

        qDebug() << "D-Bus service and object registered successfully."; // Сообщение об успешной регистрации
    }

public slots:
    // Слот, доступный через D-Bus для открытия файла
    bool OpenFile(const QString &filePath) {
        qDebug() << "Received request to open file:" << filePath; // Логирование запроса

        QFileInfo fi(filePath); // Получение информации о файле
        if (!fi.exists()) { // Проверка существования файла
            qWarning() << "File does not exist:" << filePath;
            return false; // Возврат false, если файл не существует
        }

        QString extension = fi.suffix().toLower(); // Получение и преобразование расширения файла в нижний регистр
        QString program; // Переменная для хранения имени программы, которая будет открывать файл

        // Определение приложения по расширению файла
        if (extension == "txt") {
            program = "gedit"; // Текстовый редактор для .txt файлов
        } else if (extension == "pdf") {
            program = "evince"; // Просмотрщик PDF файлов
        } else if (extension == "jpg" || extension == "png") {
            program = "eog"; // Eye of GNOME для изображений
        } else {
            // По умолчанию используем xdg-open для остальных типов файлов
            program = "xdg-open";
        }

        QStringList arguments; // Список аргументов для запуска программы
        arguments << filePath; // Добавление пути к файлу в список аргументов

        // Запуск процесса открытия файла в фоновом режиме
        bool started = QProcess::startDetached(program, arguments);
        if (started) {
            qDebug() << "Opened file with" << program; // Логирование успешного открытия файла
            return true; // Возврат true при успешном запуске процесса
        } else {
            qWarning() << "Failed to open file with" << program; // Логирование неудачи при открытии файла
            return false; // Возврат false при неудаче
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv); // Создание объекта приложения Qt без графического интерфейса

    qDebug() << "Starting FileOpener service..."; // Логирование начала работы сервиса

    FileOpener opener; // Создание экземпляра класса FileOpener, который регистрирует сервис и объект

    return app.exec(); // Запуск основного цикла приложения
}

#include "fileopener_service.moc" // Включение сгенерированного MOC файла для поддержки сигналов и слотов
