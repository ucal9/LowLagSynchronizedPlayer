#include "filtersettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QDialogButtonBox>

FilterSettingsDialog::FilterSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    updateSliders();
}

void FilterSettingsDialog::setupUI()
{
    setWindowTitle("滤镜设置");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 亮度设置
    QHBoxLayout *brightnessLayout = new QHBoxLayout;
    brightnessLabel_ = new QLabel("亮度: 0", this);
    brightnessSlider_ = new QSlider(Qt::Horizontal, this);
    brightnessSlider_->setRange(-100, 100);
    brightnessLayout->addWidget(brightnessLabel_);
    brightnessLayout->addWidget(brightnessSlider_);
    mainLayout->addLayout(brightnessLayout);

    // 对比度设置
    QHBoxLayout *contrastLayout = new QHBoxLayout;
    contrastLabel_ = new QLabel("对比度: 1.0", this);
    contrastSlider_ = new QSlider(Qt::Horizontal, this);
    contrastSlider_->setRange(0, 300);
    contrastLayout->addWidget(contrastLabel_);
    contrastLayout->addWidget(contrastSlider_);
    mainLayout->addLayout(contrastLayout);

    // 模糊设置
    QHBoxLayout *blurLayout = new QHBoxLayout;
    blurRadiusLabel_ = new QLabel("模糊: 0", this);
    blurRadiusSlider_ = new QSlider(Qt::Horizontal, this);
    blurRadiusSlider_->setRange(0, 20);
    blurLayout->addWidget(blurRadiusLabel_);
    blurLayout->addWidget(blurRadiusSlider_);
    mainLayout->addLayout(blurLayout);

    // 锐化设置
    QHBoxLayout *sharpLayout = new QHBoxLayout;
    sharpAmountLabel_ = new QLabel("锐化: 0", this);
    sharpAmountSlider_ = new QSlider(Qt::Horizontal, this);
    sharpAmountSlider_->setRange(0, 300);
    sharpLayout->addWidget(sharpAmountLabel_);
    sharpLayout->addWidget(sharpAmountSlider_);
    mainLayout->addLayout(sharpLayout);

    // 色相设置
    QHBoxLayout *hueLayout = new QHBoxLayout;
    hueLabel_ = new QLabel("色相: 0°", this);
    hueSlider_ = new QSlider(Qt::Horizontal, this);
    hueSlider_->setRange(-180, 180);
    hueLayout->addWidget(hueLabel_);
    hueLayout->addWidget(hueSlider_);
    mainLayout->addLayout(hueLayout);

    // 饱和度设置
    QHBoxLayout *saturationLayout = new QHBoxLayout;
    saturationLabel_ = new QLabel("饱和度: 1.0", this);
    saturationSlider_ = new QSlider(Qt::Horizontal, this);
    saturationSlider_->setRange(0, 300);
    saturationLayout->addWidget(saturationLabel_);
    saturationLayout->addWidget(saturationSlider_);
    mainLayout->addLayout(saturationLayout);

    // 色温设置
    QHBoxLayout *temperatureLayout = new QHBoxLayout;
    temperatureLabel_ = new QLabel("色温: 1.0", this);
    temperatureSlider_ = new QSlider(Qt::Horizontal, this);
    temperatureSlider_->setRange(0, 300);
    temperatureLayout->addWidget(temperatureLabel_);
    temperatureLayout->addWidget(temperatureSlider_);
    mainLayout->addLayout(temperatureLayout);

    // 确定取消按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    // 连接信号槽
    connect(brightnessSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onBrightnessChanged);
    connect(contrastSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onContrastChanged);
    connect(blurRadiusSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onBlurRadiusChanged);
    connect(sharpAmountSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onSharpAmountChanged);
    connect(hueSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onHueChanged);
    connect(saturationSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onSaturationChanged);
    connect(temperatureSlider_, &QSlider::valueChanged, 
            this, &FilterSettingsDialog::onTemperatureChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void FilterSettingsDialog::updateSliders()
{
    brightnessSlider_->setValue(brightness_);
    contrastSlider_->setValue(contrast_);
    blurRadiusSlider_->setValue(blur_radius_);
    sharpAmountSlider_->setValue(sharp_amount_);
    hueSlider_->setValue(hue_);
    saturationSlider_->setValue(saturation_);
    temperatureSlider_->setValue(temperature_);
}

void FilterSettingsDialog::onBrightnessChanged(int value)
{
    brightness_ = value;
    brightnessLabel_->setText(QString("亮度: %1").arg(value / 100.0));
}

void FilterSettingsDialog::onContrastChanged(int value)
{
    contrast_ = value;
    contrastLabel_->setText(QString("对比度: %1").arg(value / 100.0));
}

void FilterSettingsDialog::onBlurRadiusChanged(int value)
{
    blur_radius_ = value;
    blurRadiusLabel_->setText(QString("模糊: %1").arg(value));
}

void FilterSettingsDialog::onSharpAmountChanged(int value)
{
    sharp_amount_ = value;
    sharpAmountLabel_->setText(QString("锐化: %1").arg(value / 100.0));
}

void FilterSettingsDialog::onHueChanged(int value)
{
    hue_ = value;
    hueLabel_->setText(QString("色相: %1°").arg(value));
}

void FilterSettingsDialog::onSaturationChanged(int value)
{
    saturation_ = value;
    saturationLabel_->setText(QString("饱和度: %1").arg(value / 100.0));
}

void FilterSettingsDialog::onTemperatureChanged(int value)
{
    temperature_ = value;
    temperatureLabel_->setText(QString("色温: %1").arg(value / 100.0));
}

// 实现设置参数的方法
void FilterSettingsDialog::setBrightness(float value)
{
    brightness_ = value;
    brightnessSlider_->setValue(value);
}

void FilterSettingsDialog::setContrast(float value)
{
    contrast_ = value;
    contrastSlider_->setValue(value);
}

void FilterSettingsDialog::setBlurRadius(int value)
{
    blur_radius_ = value;
    blurRadiusSlider_->setValue(value);
}

void FilterSettingsDialog::setSharpAmount(float value)
{
    sharp_amount_ = value;
    sharpAmountSlider_->setValue(value);
}

void FilterSettingsDialog::setHue(float value)
{
    hue_ = value;
    hueSlider_->setValue(value);
}

void FilterSettingsDialog::setSaturation(float value)
{
    saturation_ = value;
    saturationSlider_->setValue(value);
}

void FilterSettingsDialog::setTemperature(float value)
{
    temperature_ = value;
    temperatureSlider_->setValue(value);
} 