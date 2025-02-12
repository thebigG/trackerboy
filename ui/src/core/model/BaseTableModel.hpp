#pragma once

#include "core/Module.hpp"

#include "trackerboy/data/Table.hpp"

#include <QAbstractListModel>
#include <QIcon>

//
// Base class for the WaveListModel and InstrumentListModel.
//
class BaseTableModel : public QAbstractListModel {

    Q_OBJECT

public:

    virtual ~BaseTableModel();

    bool canAdd() const;

    // resets the model. called when the document is reset or loaded a
    // module from a file
    void reload();

    // commit all string data to the underlying data table. To be called
    // before saving the module
    void commit();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    // adds a new item, if there was no items prior then this one is selected
    int add();
    // removes the given item
    void remove(int index);

    int duplicate(int index);

    QString name(int index);

    uint8_t id(int index);

    int lookupId(uint8_t id);

    void rename(int index, const QString &name);


protected:
    BaseTableModel(Module &mod, trackerboy::BaseTable& table, QString defaultName, QObject *parent = nullptr);

    virtual QIcon iconData(uint8_t id) const = 0;

    Module &mModule;
    trackerboy::BaseTable& mBaseTable;

private:
    Q_DISABLE_COPY(BaseTableModel)

    // a QString copy of each name in table is stored in this model
    // this way we don't have to convert to and from std::string when
    // displaying/editing names. The conversion only occurs on reload
    // and commit
    struct ModelData {

        ModelData(uint8_t id, QString name);
        explicit ModelData(trackerboy::DataItem const& item);

        uint8_t id;
        QString name;


    };

    int insertData(ModelData const& data);

    // maps a model index -> table index
    std::vector<ModelData> mItems;

    QString const mDefaultName;
    bool mShouldCommit;

    
};
