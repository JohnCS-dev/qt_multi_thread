#ifndef ISTORAGE_H
#define ISTORAGE_H

#include <QObject>

#define PORT 1024

class IStorage
{
public:
    enum Operation {
        opRead,
        opWrite,
        opCount
    };
    virtual ~IStorage() = default;
    virtual void writeItem(int index, const int & item) = 0;
    virtual int readItem(int index) = 0;
    virtual void printStatistics() = 0;
};

#endif // ISTORAGE_H
