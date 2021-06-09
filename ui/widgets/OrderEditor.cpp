
#include "widgets/OrderEditor.hpp"

#include "misc/IconManager.hpp"
#include "misc/utils.hpp"

#include <QFontDatabase>
#include <QHeaderView>
#include <QtDebug>


OrderEditor::OrderEditor(QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mIgnoreSelect(false),
    mContextMenu(),
    mLayout(),
    mToolbar(),
    mSetButton(),
    mOrderView()
{

    // layout
    mLayoutSet.addWidget(&mToolbar);
    mLayoutSet.addStretch(1);
    mLayoutSet.addWidget(&mSetSpin);
    mLayoutSet.addWidget(&mSetButton);

    mLayout.addLayout(&mLayoutSet);
    mLayout.addWidget(&mOrderView, 1);
    setLayout(&mLayout);
    
    // settings

    mToolbar.setIconSize(QSize(16, 16));
    mToolbar.addAction(&mActionInsert);
    mToolbar.addAction(&mActionRemove);
    mToolbar.addAction(&mActionDuplicate);
    mToolbar.addAction(&mActionMoveUp);
    mToolbar.addAction(&mActionMoveDown);
    mToolbar.addAction(&mActionIncrement);
    mToolbar.addAction(&mActionDecrement);

    mSetSpin.setDigits(2);
    mSetSpin.setRange(0, 255);
    mSetSpin.setDisplayIntegerBase(16);
    
    mSetButton.setText(tr("Set"));
    mSetButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    
    mActionRemove.setEnabled(false);
    mActionMoveUp.setEnabled(false);
    mActionMoveDown.setEnabled(false);

    // actions
    setupAction(mActionInsert, "&Insert order row", "Inserts a new order at the current pattern", Icons::itemAdd);
    setupAction(mActionRemove, "&Remove order row", "Removes the order at the current pattern", Icons::itemRemove);
    setupAction(mActionDuplicate, "&Duplicate order row", "Duplicates the order at the current pattern", Icons::itemDuplicate);
    setupAction(mActionMoveUp, "Move order &up", "Moves the order up 1", Icons::moveUp);
    setupAction(mActionMoveDown, "Move order dow&n", "Moves the order down 1", Icons::moveDown);
    setupAction(mActionIncrement, "Increment selection", "Increments all selected cells by 1", Icons::increment);
    setupAction(mActionDecrement, "Decrement selection", "Decrements all selected cells by 1", Icons::decrement);

    connect(&mActionInsert, &QAction::triggered, this, &OrderEditor::insert);
    connect(&mActionRemove, &QAction::triggered, this, &OrderEditor::remove);
    connect(&mActionDuplicate, &QAction::triggered, this, &OrderEditor::duplicate);
    connect(&mActionMoveUp, &QAction::triggered, this, &OrderEditor::moveUp);
    connect(&mActionMoveDown, &QAction::triggered, this, &OrderEditor::moveDown);

    // menu
    setupMenu(mContextMenu);

    mOrderView.setTabKeyNavigation(false);
    mOrderView.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(&mOrderView, &QTableView::customContextMenuRequested, this, &OrderEditor::tableViewContextMenu);
    
    auto headerView = mOrderView.horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);


    auto verticalHeader = mOrderView.verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

    connect(&mActionIncrement, &QAction::triggered, this, &OrderEditor::increment);
    connect(&mActionDecrement, &QAction::triggered, this, &OrderEditor::decrement);
    connect(&mSetButton, &QToolButton::clicked, this, &OrderEditor::set);

    setEnabled(false);
}

OrderEditor::~OrderEditor() {
}

void OrderEditor::setupMenu(QMenu &menu) {
    menu.addAction(&mActionInsert);
    menu.addAction(&mActionRemove);
    menu.addAction(&mActionDuplicate);
    menu.addSeparator();
    menu.addAction(&mActionMoveUp);
    menu.addAction(&mActionMoveDown);
}

void OrderEditor::setDocument(ModuleDocument *doc) {

    // disconnect all signals with the model
    if (mDocument != nullptr) {
        mDocument->state().orderSetSpinbox = mSetSpin.value();
        mDocument->orderModel().disconnect(this);
    }

    bool const enabled = doc != nullptr;
    

    mDocument = doc;

    if (enabled) {
        auto &model = doc->orderModel();
        mOrderView.setModel(&model);
        connect(&model, &OrderModel::currentIndexChanged, this,
        [this](const QModelIndex &index) {
            if (!mIgnoreSelect) {
                mOrderView.selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
            }
        });

        auto selectionModel = mOrderView.selectionModel();
        connect(selectionModel, &QItemSelectionModel::currentChanged, this, &OrderEditor::currentChanged);
        connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &OrderEditor::selectionChanged);
        selectionModel->select(model.currentIndex(), QItemSelectionModel::Select);

        // restore ui state
        mSetSpin.setValue(doc->state().orderSetSpinbox);
    } else {
        mOrderView.setModel(nullptr);
    }

    setEnabled(enabled);
    
}

void OrderEditor::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    Q_UNUSED(prev);

    if (mDocument) {
        mIgnoreSelect = true;
        mDocument->orderModel().select(current.row(), current.column());
        mIgnoreSelect = false;
    }
}

void OrderEditor::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if (mDocument) {
        // this slot is just for preventing the user from deselecting
        auto model = mOrderView.selectionModel();
        if (!model->hasSelection()) {
            // user deselected everything, force selection of the current index
            model->select(mOrderView.currentIndex(), QItemSelectionModel::Select);
        }
    }
}

void OrderEditor::increment() {
    mDocument->orderModel().incrementSelection(mOrderView.selectionModel()->selection());
}

void OrderEditor::decrement() {
    mDocument->orderModel().decrementSelection(mOrderView.selectionModel()->selection());
}

void OrderEditor::set() {
    mDocument->orderModel().setSelection(
        mOrderView.selectionModel()->selection(),
        static_cast<uint8_t>(mSetSpin.value())
        );
}

void OrderEditor::tableViewContextMenu(QPoint pos) {
    mContextMenu.popup(mOrderView.viewport()->mapToGlobal(pos));
}

void OrderEditor::insert() {
    mDocument->orderModel().insert();
    updateActions();
}

void OrderEditor::remove() {
    mDocument->orderModel().remove();
    updateActions();
}

void OrderEditor::duplicate() {
    mDocument->orderModel().duplicate();
    updateActions();
}

void OrderEditor::moveUp() {
    mDocument->orderModel().moveUp();
    updateActions();
}

void OrderEditor::moveDown() {
    mDocument->orderModel().moveDown();
    updateActions();
}

void OrderEditor::updateActions() {
    auto &model = mDocument->orderModel();
    bool canInsert = model.canInsert();
    mActionInsert.setEnabled(canInsert);
    mActionDuplicate.setEnabled(canInsert);
    mActionRemove.setEnabled(model.canRemove());
    mActionMoveUp.setEnabled(model.canMoveUp());
    mActionMoveDown.setEnabled(model.canMoveDown());
}