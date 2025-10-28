#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

class KalmanFilter
{
private:
    float x; // estimate
    float p; // error estimate
    float r; // error measure
    float k; // kalman gain
    float a; //Hệ số làm mượt độ nhiễu cảm biến

public:
    KalmanFilter(float init_estimate = 0.0, float init_error_estimate = 1.0, float error_measure = 1.0, float alpha = 0.9);
    float update(float measurement);
    float getEstimate() const;
    float getErrorMeasure() const;
};

#endif
