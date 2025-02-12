
#pragma once

#include "widgets/config/ConfigTab.hpp"
#include "widgets/config/SoundQualityPreview.hpp"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

//
// Tab widget for the "Sound" tab in ConfigDialog
//
class SoundConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit SoundConfigTab(QWidget *parent = nullptr);
    ~SoundConfigTab() = default;

    void apply(SoundConfig &soundConfig);

    void resetControls(SoundConfig const& soundConfig);

private slots:

    void qualityRadioToggled(QAbstractButton *btn, bool checked);

    void rescan(bool rescanDueToApiChange = false);

private:
    Q_DISABLE_COPY(SoundConfigTab)

    void setupTimeSpinbox(QSpinBox &spin);

    void onDeviceComboSelected(int index);

    void populateDevices();

    void apiChanged(int index);

    QVBoxLayout mLayout;
        QGroupBox mDeviceGroup;
            QGridLayout mDeviceLayout;
                // row 0
                QLabel mApiLabel;
                QComboBox mApiCombo;
                // row 1
                QLabel mDeviceLabel;
                QComboBox mDeviceCombo;
                // row 2
                QHBoxLayout mRescanLayout;
                    QPushButton mRescanButton;
                // row 3
                QLabel mLatencyLabel;
                QSpinBox mLatencySpin;
                // row 4
                QLabel mPeriodLabel;
                QSpinBox mPeriodSpin;
                // row 5
                QLabel mSamplerateLabel;
                QComboBox mSamplerateCombo;
        QGroupBox mQualityGroup;
            QVBoxLayout mQualityLayout;
                QHBoxLayout mQualityRadioLayout;
                    QRadioButton mLowQualityRadio;
                    QRadioButton mMedQualityRadio;
                    QRadioButton mHighQualityRadio;
                QGridLayout mPreviewLayout;
                    QLabel mChannels12Label;        // 0, 0
                    QLabel mChannels34Label;        // 0, 1
                    SoundQualityPreview mPreview12; // 1, 0
                    SoundQualityPreview mPreview34; // 1, 1

    QButtonGroup mQualityButtons;

};
