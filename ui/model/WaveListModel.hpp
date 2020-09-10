
#pragma once

#include <QAbstractListModel>

#include "trackerboy/data/Table.hpp"

#include "model/BaseTableModel.hpp"

// Model for QListView (Waveforms dock widget) or QComboBox
// used by QComboBox for selecting a waveform in the Instrument editor or Wave editor
class WaveListModel : public BaseTableModel {

    Q_OBJECT

public:
    WaveListModel(trackerboy::WaveTable &table, QObject *parent = nullptr);

    // get the waveform associated with the model index
    trackerboy::Waveform* waveform(int modelIndex);

    trackerboy::Waveform* currentWaveform();

signals:
    // emitted when the current waveform's data was modified
    void waveformChanged();
    // same as above, but only for a sample index
    void waveformChanged(QPoint point);

    // editing
    // these methods edit the current waveform

public slots:
    void setSample(QPoint point);
    void setData(uint8_t *wavedata);
    void setData(const QString &text);

    void clear();
    void invert();
    void rotateLeft();
    void rotateRight();

    
protected:
    QVariant iconData(const QModelIndex &index) const override;


};
