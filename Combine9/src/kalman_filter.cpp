#include "kalman_filter.h"
#include <cmath>

KalmanFilter::KalmanFilter(float init_estimate, float init_error_estimate, float init_error_measure, float alpha) {
    x = init_estimate;
    p = init_error_estimate;
    r = init_error_measure;
    a = alpha;
}
float KalmanFilter::update(float measurement) {
    // Tính sai số giữa đo và dự đoán
    float residual = fabs(measurement - x); // lấy số dương

    // Cập nhật độ nhiễu cảm biến một cách mượt mà
    r = a * r + (1 - a) * residual;

    // Tính Kalman Gain
    k = p / (p + r);

    // Cập nhật giá trị ước lượng
    x = x + k * (measurement - x);

    // Cập nhật độ tin cậy của ước lượng
    p = (1 - k) * p;

    return x;
}

float KalmanFilter::getEstimate() const {
    return x;
}

float KalmanFilter::getErrorMeasure() const {
    return p;
}
