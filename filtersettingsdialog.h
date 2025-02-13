#ifndef FILTERSETTINGSDIALOG_H
#define FILTERSETTINGSDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

class FilterSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterSettingsDialog(QWidget *parent = nullptr);

    // 获取参数的方法
    float getBrightness() const { return brightness_; }
    float getContrast() const { return contrast_; }
    int getBlurRadius() const { return blur_radius_; }
    float getSharpAmount() const { return sharp_amount_; }
    float getHue() const { return hue_; }
    float getSaturation() const { return saturation_; }
    float getTemperature() const { return temperature_; }

    // 设置参数的方法
    void setBrightness(float value);
    void setContrast(float value);
    void setBlurRadius(int value);
    void setSharpAmount(float value);
    void setHue(float value);
    void setSaturation(float value);
    void setTemperature(float value);

private slots:
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onBlurRadiusChanged(int value);
    void onSharpAmountChanged(int value);
    void onHueChanged(int value);
    void onSaturationChanged(int value);
    void onTemperatureChanged(int value);

private:
    void setupUI();
    void updateSliders();

    QSlider *brightnessSlider_;
    QSlider *contrastSlider_;
    QSlider *blurRadiusSlider_;
    QSlider *sharpAmountSlider_;
    QSlider *hueSlider_;
    QSlider *saturationSlider_;
    QSlider *temperatureSlider_;

    QLabel *brightnessLabel_;
    QLabel *contrastLabel_;
    QLabel *blurRadiusLabel_;
    QLabel *sharpAmountLabel_;
    QLabel *hueLabel_;
    QLabel *saturationLabel_;
    QLabel *temperatureLabel_;

    int brightness_ = 0;     // -100 到 100
    int contrast_ = 50;      // 0 到 100
    int blur_radius_ = 0;    // 0 到 10
    int sharp_amount_ = 0;   // 0 到 100
    int hue_ = 0;           // -180 到 180
    int saturation_ = 100;  // 0 到 200
    int temperature_ = 100; // 0 到 200
};

#endif // FILTERSETTINGSDIALOG_H 