class Display : public QWidget
{
    Q_OBJECT
public:
    // ... existing code ...
    
    void setFilterParams(float brightness, 
                        float contrast,
                        int blurRadius,
                        float sharpAmount,
                        float hue,
                        float saturation,
                        float temperature);

private:
    // ... existing code ...
    
    // 滤镜参数
    float brightness_ = 0.0f;
    float contrast_ = 1.0f;
    int blurRadius_ = 0;
    float sharpAmount_ = 0.0f;
    float hue_ = 0.0f;
    float saturation_ = 1.0f;
    float temperature_ = 1.0f;
}; 