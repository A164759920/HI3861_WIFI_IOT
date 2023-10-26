// storage.h

#ifndef STORAGE_H
#define STORAGE_H

struct SensorData
{
    double temp;
    double humi;
};
void generateSensorData_thread(void *arg);
#endif
