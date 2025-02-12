
#include "widgets/docks/BaseEditor.hpp"

#include <QSignalBlocker>
#include <QHBoxLayout>
#include <QLabel>
#include <QtDebug>

BaseEditor::BaseEditor(
    BaseTableModel &model,
    PianoInput const& input, 
    QWidget *parent
) :
    QWidget(parent),
    mModel(model)
{

    mCombo = new QComboBox;
    mNameEdit = new QLineEdit;
    mEditorWidget = new QWidget;
    mPiano = new PianoWidget(input);

    auto tableLayout = new QHBoxLayout;
    tableLayout->addWidget(mCombo, 1);
    tableLayout->addWidget(new QLabel(tr("Name:")));
    tableLayout->addWidget(mNameEdit, 1);

    auto layout = new QVBoxLayout;
    layout->addLayout(tableLayout);
    layout->addWidget(mEditorWidget, 1);
    layout->addWidget(mPiano);
    setLayout(layout);

    mCombo->setModel(&model);

    connect(mCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &BaseEditor::onIndexChanged);
    connect(mNameEdit, &QLineEdit::textEdited, this, &BaseEditor::onNameEdited);

    connect(&model, &BaseTableModel::modelReset, this,
        [this]() {
            int index = mModel.rowCount() > 0 ? 0 : -1;
            QSignalBlocker blocker(mCombo);
            mCombo->setCurrentIndex(index);
            onIndexChanged(index);
        });

    setEnabled(false);
}

PianoWidget* BaseEditor::piano() {
    return mPiano;
}

void BaseEditor::openItem(int index) {
    mCombo->setCurrentIndex(index);
}

QWidget* BaseEditor::editorWidget() {
    return mEditorWidget;
}

int BaseEditor::currentItem() const {
    return mCombo->currentIndex();
}

void BaseEditor::onIndexChanged(int index) {

    bool const hasIndex = index != -1;
    if (hasIndex) {
        mNameEdit->setText(mModel.name(index));
    } else {
        mNameEdit->clear();
    }
    setEnabled(hasIndex);
    setCurrentItem(index);
}

void BaseEditor::onNameEdited(QString const& name) {
    mModel.rename(mCombo->currentIndex(), name);
}

