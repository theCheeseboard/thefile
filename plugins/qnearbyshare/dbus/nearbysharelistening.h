#ifndef NEARBYSHARELISTENING_H
#define NEARBYSHARELISTENING_H

#include <QObject>

class NearbyShareListening : public QObject
{
    Q_OBJECT
public:
    explicit NearbyShareListening(QObject *parent = nullptr);

signals:

};

#endif // NEARBYSHARELISTENING_H
