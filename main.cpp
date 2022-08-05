#include <QCoreApplication>

#include <chrono>
#include <QObject>
#include <QTimer>
#include <QDebug>

#include "cosignal.h"

class TestObj
    : public QObject
{
    Q_OBJECT

    int counter{ 0 };

signals:
    void valueReady(int value);
    void error(int errCode, QString errStr);

public:
    TestObj(QObject* parent = nullptr)
        : QObject{ parent }
    {
        QTimer* t1 = new QTimer{ this };
        QObject::connect(t1, &QTimer::timeout, this, [this]{
            ++counter;
            emit valueReady(counter);
        }, Qt::QueuedConnection);
        t1->start(std::chrono::milliseconds(300));

        QTimer* t2 = new QTimer{ this };
        QObject::connect(t2, &QTimer::timeout, this, [this]{
            emit error(42, "Because fuck you... That's why");
        }, Qt::QueuedConnection);
        t2->start(std::chrono::milliseconds(1000));
    }
};

Cosignal test_cosignal(TestObj* emitter)
{
    qDebug() << "CORO START";
    bool stop = false;
    while (!stop) {
        SWITCH(
            CASE(emitter, &TestObj::valueReady)(int value){
                qDebug() << "CORO GOT VALUE:" << value;
            }};
            CASE(emitter, &TestObj::error)(int errCode, QString errStr){
                qDebug() << "CORO GOT ERROR:" << "CODE:" << errCode << "STR:" << errStr;
                stop = true;
            }};
        )
    }
    QCoreApplication::quit();
    qDebug() << "CORO FINISH";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestObj testObj;

    qDebug() << "MAIN BEFORE CORO START";
    test_cosignal(&testObj);
    qDebug() << "MAIN AFTER CORO START";

    return a.exec();
}

#include "main.moc"
