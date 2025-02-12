
#include "MainWindow.hpp"

#include "misc/IconManager.hpp"
#include "misc/connectutils.hpp"
#include "widgets/docks/InstrumentEditor.hpp"
#include "widgets/docks/TableDock.hpp"
#include "widgets/docks/WaveEditor.hpp"

#include <QApplication>
#include <QSettings>
#include <QShortcut>
#include <QScreen>
#include <QMenu>
#include <QStatusBar>
#include <QtDebug>
#include <QUndoView>

#define TU MainWindowTU

namespace TU {

static auto const KEY_MAIN_WINDOW = QStringLiteral("MainWindow");
static auto const KEY_WINDOW_STATE = QStringLiteral("windowState");
static auto const KEY_GEOMETRY = QStringLiteral("geometry");

}

MainWindow::MainWindow() :
    QMainWindow(),
    mUntitledString(tr("Untitled")),
    mConfig(),
    mMidi(),
    mMidiReceiver(nullptr),
    mMidiNoteDown(false),
    mModule(),
    mModuleFile(),
    mErrorSinceLastConfig(false),
    mAboutDialog(nullptr),
    mAudioDiag(nullptr),
    mConfigDialog(nullptr),
    mTempoCalc(nullptr)
{

    // create models
    mModule = new Module(this);
    mInstrumentModel = new InstrumentListModel(*mModule, this);
    mSongModel = new SongModel(*mModule, this);
    mPatternModel = new PatternModel(*mModule, *mSongModel, this);
    mWaveModel = new WaveListModel(*mModule, this);

    mRenderer = new Renderer(*mModule, this);

    setupUi();

    // read in application configuration
    mConfig.readSettings();
    
    setWindowIcon(IconManager::getAppIcon());

    QSettings settings;
    settings.beginGroup(TU::KEY_MAIN_WINDOW);

    // restore geomtry from the last session
    auto const geometry = settings.value(TU::KEY_GEOMETRY, QByteArray()).toByteArray();

    if (geometry.isEmpty()) {
        // no saved geometry, initialize it
        // we will take 3/4 of the primary screen's width and height, but we
        // will take no more than 1280x720

        // maximum initial resolution
        constexpr int MAX_WIDTH = 1280;
        constexpr int MAX_HEIGHT = 720;

        // get the available geometry for the primary screen
        auto const availableGeometry = QApplication::primaryScreen()->availableGeometry();

        QRect newGeometry(
            0,
            0, 
            std::min(MAX_WIDTH, availableGeometry.width() * 3 / 4),
            std::min(MAX_HEIGHT, availableGeometry.height() * 3 / 4)
        );

        newGeometry.moveTo(availableGeometry.center() - newGeometry.center());
        setGeometry(newGeometry);
    } else {
        restoreGeometry(geometry);
    }

    // restore window state if it exists
    auto const windowState = settings.value(TU::KEY_WINDOW_STATE).toByteArray();
    if (windowState.isEmpty()) {
        // default layout
        initState();
    } else {
        addToolBar(mToolbarFile);
        addToolBar(mToolbarEdit);
        addToolBar(mToolbarSong);
        addToolBar(mToolbarTracker);
        addToolBar(mToolbarInput);
        addDockWidget(Qt::LeftDockWidgetArea, mDockInstrumentEditor);
        addDockWidget(Qt::LeftDockWidgetArea, mDockWaveformEditor);
        addDockWidget(Qt::LeftDockWidgetArea, mDockHistory);
        addDockWidget(Qt::LeftDockWidgetArea, mDockInstruments);
        addDockWidget(Qt::LeftDockWidgetArea, mDockWaveforms);
        restoreState(windowState);
    }

    mModuleFile.setName(mUntitledString);
    updateWindowTitle();

    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);

}

MainWindow::~MainWindow() {

}

QMenu* MainWindow::createPopupMenu() {
    auto menu = new QMenu(this);
    setupViewMenu(menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        // user saved or discarded changes, close the window
        #ifdef QT_DEBUG
        if (mSaveConfig) {
        #endif
            QSettings settings;
            settings.beginGroup(TU::KEY_MAIN_WINDOW);
            settings.setValue(TU::KEY_GEOMETRY, saveGeometry());
            settings.setValue(TU::KEY_WINDOW_STATE, saveState());
        #ifdef QT_DEBUG
        }
        #endif
        evt->accept();
    } else {
        // user aborted closing, ignore this event
        evt->ignore();
    }
    
}

void MainWindow::showEvent(QShowEvent *evt) {
    Q_UNUSED(evt)
    mPatternEditor->setFocus();
}

// PRIVATE METHODS -----------------------------------------------------------

bool MainWindow::maybeSave() {
    if (mModule->isModified()) {
        // prompt the user if they want to save any changes
        auto const result = QMessageBox::warning(
            this,
            tr("Trackerboy"),
            tr("Save changes to %1?").arg(mModuleFile.name()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        switch (result) {
            case QMessageBox::Save:
                if (!onFileSave()) {
                    // save failed, do not close document
                    return false;
                }
                break;
            case QMessageBox::Cancel:
                // user cancelled, do not close document
                return false;
            default:
                break;
        }
    }

    return true;
}

void MainWindow::commitModels() {
    // libtrackerboy uses std::string, models use QString
    // models use a QString version of the underlying std::string data
    // commiting converts the QString data (if needed) to std::string
    mInstrumentModel->commit();
    mWaveModel->commit();
}

QDockWidget* MainWindow::makeDock(QString const& title, QString const& objname) {
    auto dock = new QDockWidget(title, this);
    dock->setObjectName(objname);
    return dock;
}

QToolBar* MainWindow::makeToolbar(QString const& title, QString const& objname) {
    auto toolbar = new QToolBar(title, this);
    toolbar->setObjectName(objname);
    toolbar->setIconSize(IconManager::size());
    return toolbar;
}

void MainWindow::setupUi() {

    // CENTRAL WIDGET ========================================================

    auto centralWidget = new QWidget(this);
    auto layout = new QHBoxLayout;
    mSidebar = new Sidebar;
    mPatternEditor = new PatternEditor(mConfig.pianoInput(), *mPatternModel);
    layout->addWidget(mSidebar);
    layout->addWidget(mPatternEditor, 1);
    centralWidget->setLayout(layout);

    //mSidebar->orderEditor()->setModel(mOrderModel);
    mSidebar->songEditor()->setModel(mSongModel);

    setCentralWidget(centralWidget);

    {
        auto grid = mPatternEditor->grid();
        grid->setFirstHighlight(mSongModel->rowsPerBeat());
        grid->setSecondHighlight(mSongModel->rowsPerMeasure());
        lazyconnect(mSongModel, rowsPerBeatChanged, grid, setFirstHighlight);
        lazyconnect(mSongModel, rowsPerMeasureChanged, grid, setSecondHighlight);
    }

    // DOCKS =================================================================

    mDockHistory = makeDock(tr("History"), QStringLiteral("DockHistory"));
    auto undoView = new QUndoView(mModule->undoGroup(), mDockHistory);
    mDockHistory->setWidget(undoView);

    mDockInstruments = makeDock(tr("Instruments"), QStringLiteral("DockInstruments"));
    auto instruments = new TableDock(*mInstrumentModel, tr("Ctrl+I"), tr("instrument"), mDockInstruments);
    connect(instruments, &TableDock::selectedItemChanged, this,
        [this](int index) {
            int id = -1;
            if (index != -1) {
                id = mInstrumentModel->id(index);
            }
            mPatternEditor->setInstrument(id);
        });
    mDockInstruments->setWidget(instruments);

    mDockWaveforms = makeDock(tr("Waveforms"), QStringLiteral("DockWaveforms"));
    auto waveforms = new TableDock(*mWaveModel, tr("Ctrl+W"), tr("waveform"), mDockWaveforms);
    mDockWaveforms->setWidget(waveforms);

    mDockInstrumentEditor = makeDock(tr("Instrument editor"), QStringLiteral("DockInstrumentEditor"));
    auto instrumentEditor = new InstrumentEditor(
        *mModule,
        *mInstrumentModel,
        *mWaveModel,
        mConfig.pianoInput(),
        mDockInstrumentEditor
    );
    mDockInstrumentEditor->setWidget(instrumentEditor);

    mDockWaveformEditor = makeDock(tr("Waveform editor"), QStringLiteral("DockWaveformEditor"));
    auto waveEditor = new WaveEditor(
        *mModule,
        *mWaveModel,
        mConfig.pianoInput(),
        mDockWaveformEditor
    );
    mDockWaveformEditor->setWidget(waveEditor);

    // TOOLBARS ==============================================================

    mToolbarFile = makeToolbar(tr("File"), QStringLiteral("ToolbarFile"));
    mToolbarEdit = makeToolbar(tr("Edit"), QStringLiteral("ToolbarEdit"));
    mToolbarSong = makeToolbar(tr("Song"), QStringLiteral("ToolbarSong"));
    mToolbarTracker = makeToolbar(tr("Tracker"), QStringLiteral("ToolbarTracker"));
    mToolbarInput = makeToolbar(tr("Input"), QStringLiteral("ToolbarInput"));

    auto octaveSpin = new QSpinBox(mToolbarInput);
    octaveSpin->setRange(2, 8);
    octaveSpin->setValue(mConfig.pianoInput().octave());
    connect(octaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, 
        [this](int octave) {
            mConfig.pianoInput().setOctave(octave);
        });
    auto editStepSpin = new QSpinBox(mToolbarInput);
    editStepSpin->setRange(0, 255);
    editStepSpin->setValue(1);
    connect(editStepSpin, qOverload<int>(&QSpinBox::valueChanged), mPatternEditor, &PatternEditor::setEditStep);
    mToolbarInput->addWidget(new QLabel(tr("Octave"), mToolbarInput));
    mToolbarInput->addWidget(octaveSpin);
    mToolbarInput->addWidget(new QLabel(tr("Edit step"), mToolbarInput));
    mToolbarInput->addWidget(editStepSpin);
    mToolbarInput->addSeparator();

    // ACTIONS ===============================================================

    createActions(instruments->tableActions(), waveforms->tableActions());

    mToolbarSong->addAction(mActionOrderInsert);
    mToolbarSong->addAction(mActionOrderRemove);
    mToolbarSong->addAction(mActionOrderDuplicate);
    mToolbarSong->addAction(mActionOrderMoveUp);
    mToolbarSong->addAction(mActionOrderMoveDown);

    // SHORTCUTS =============================================================

    QShortcut *shortcut;

    shortcut = new QShortcut(tr("Ctrl+Left"), this);
    lazyconnect(shortcut, activated, this, previousInstrument);

    shortcut = new QShortcut(tr("Ctrl+Right"), this);
    lazyconnect(shortcut, activated, this, nextInstrument);

    shortcut = new QShortcut(tr("Ctrl+Up"), this);
    lazyconnect(shortcut, activated, this, previousPattern);
    
    shortcut = new QShortcut(tr("Ctrl+Down"), this);
    lazyconnect(shortcut, activated, this, nextPattern);

    shortcut = new QShortcut(QKeySequence(Qt::KeypadModifier | Qt::Key_Asterisk), this);
    lazyconnect(shortcut, activated, this, increaseOctave);
    
    shortcut = new QShortcut(QKeySequence(Qt::KeypadModifier | Qt::Key_Slash), this);
    lazyconnect(shortcut, activated, this, decreaseOctave);

    shortcut = new QShortcut(QKeySequence(Qt::Key_Return), mPatternEditor);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    lazyconnect(shortcut, activated, this, playOrStop);

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();

    mStatusRenderer = new QLabel(statusbar);
    mStatusSpeed = new SpeedLabel(statusbar);
    mStatusTempo = new TempoLabel(statusbar);
    mStatusElapsed = new QLabel(statusbar);
    mStatusPos = new QLabel(statusbar);
    mStatusSamplerate = new QLabel(statusbar);

    mStatusRenderer->setMinimumWidth(60);
    mStatusSpeed->setMinimumWidth(60);
    mStatusTempo->setMinimumWidth(60);
    mStatusElapsed->setMinimumWidth(40);
    mStatusPos->setMinimumWidth(40);
    mStatusSamplerate->setMinimumWidth(60);

    for (auto label : { 
            mStatusRenderer,
            (QLabel*)mStatusSpeed,
            (QLabel*)mStatusTempo,
            mStatusElapsed,
            mStatusPos,
            mStatusSamplerate
            }) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        statusbar->addPermanentWidget(label);
    }

    statusbar->showMessage(tr("Trackerboy v%1.%2.%3")
        .arg(trackerboy::VERSION.major)
        .arg(trackerboy::VERSION.minor)
        .arg(trackerboy::VERSION.patch));

    // default statuses
    setPlayingStatus(PlayingStatusText::ready);
    mStatusElapsed->setText(QStringLiteral("00:00"));
    mStatusPos->setText(QStringLiteral("00 / 00"));
    // no need to set samplerate, it is done so in onConfigApplied

    // CONNECTIONS ============================================================

    // editors
    {
        auto piano = instrumentEditor->piano();
        connect(piano, &PianoWidget::keyDown, this,
            [this, instrumentEditor](int note) {
                auto item = instrumentEditor->currentItem();
                if (item != -1) {
                    mRenderer->instrumentPreview(note, -1, mInstrumentModel->id(item));
                }
            });
        lazyconnect(piano, keyChange, mRenderer, setPreviewNote);
        lazyconnect(piano, keyUp, mRenderer, stopPreview);
    }

    connect(instrumentEditor, &InstrumentEditor::openWaveEditor, this,
        [this](int index) {
            openEditor(mDockWaveformEditor, index);
        });

    {
        auto piano = waveEditor->piano();
        connect(piano, &PianoWidget::keyDown, this,
            [this, waveEditor](int note) {
                auto item = waveEditor->currentItem();
                if (item != -1) {
                    mRenderer->waveformPreview(note, mWaveModel->id(item));
                }
            });
        lazyconnect(piano, keyChange, mRenderer, setPreviewNote);
        lazyconnect(piano, keyUp, mRenderer, stopPreview);
    }

    auto orderEditor = mSidebar->orderEditor();
    connect(orderEditor, &OrderEditor::popupMenuAt, this,
        [this](QPoint const& pos) {
            if (mSongOrderContextMenu == nullptr) {
                mSongOrderContextMenu = new QMenu(this);
                setupSongMenu(mSongOrderContextMenu);
            }
            mSongOrderContextMenu->popup(pos);
        });
    lazyconnect(orderEditor, jumpToPattern, mRenderer, jumpToPattern);

    lazyconnect(mPatternEditor, previewNote, mRenderer, instrumentPreview);
    lazyconnect(mPatternEditor, stopNotePreview, mRenderer, stopPreview);

    connect(&mMidi, &Midi::error, this,
        [this]() {
            disableMidi(true);
        });

    connect(&mMidi, &Midi::noteOn, this,
        [this](int note) {
            mMidiReceiver->midiNoteOn(note);
            mMidiNoteDown = true;
        });

    connect(&mMidi, &Midi::noteOff, this,
        [this]() {
            mMidiReceiver->midiNoteOff();
            mMidiNoteDown = false;
        });

    connect(mModule, &Module::modifiedChanged, this, &MainWindow::setWindowModified);

    //connect(mOrderModel, &OrderModel::currentPatternChanged, this, &MainWindow::updateOrderActions);
    updateOrderActions();

    connect(mRenderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart);
    connect(mRenderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop);
    connect(mRenderer, &Renderer::audioError, this, &MainWindow::onAudioError);
    connect(mRenderer, &Renderer::frameSync, this, &MainWindow::onFrameSync);
    
    auto scope = mSidebar->scope();
    scope->setBuffer(&mRenderer->visualizerBuffer());
    connect(mRenderer, &Renderer::updateVisualizers, scope, qOverload<>(&AudioScope::update));

    lazyconnect(mRenderer, isPlayingChanged, mPatternModel, setPlaying);

    connect(instruments, &TableDock::edit, this,
        [this](int item) {
            openEditor(mDockInstrumentEditor, item);
        });
    connect(waveforms, &TableDock::edit, this,
        [this](int item) {
            openEditor(mDockWaveformEditor, item);
        });

    connect(mPatternEditor->gridHeader(), &PatternGridHeader::outputChanged, mRenderer, &Renderer::setChannelOutput);

    auto app = static_cast<QApplication*>(QApplication::instance());
    connect(app, &QApplication::focusChanged, this, &MainWindow::handleFocusChange);

}

void MainWindow::initState() {
    // setup default layout

    // setup corners, left and right get both corners
    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);
    setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);


    addToolBar(Qt::TopToolBarArea, mToolbarFile);
    mToolbarFile->show();

    addToolBar(Qt::TopToolBarArea, mToolbarEdit);
    mToolbarEdit->show();

    addToolBar(Qt::TopToolBarArea, mToolbarSong);
    mToolbarSong->show();

    addToolBar(Qt::TopToolBarArea, mToolbarTracker);
    mToolbarTracker->show();

    addToolBar(Qt::TopToolBarArea, mToolbarInput);
    mToolbarInput->show();

    addDockWidget(Qt::RightDockWidgetArea, mDockInstrumentEditor);
    mDockInstrumentEditor->setFloating(true);
    mDockInstrumentEditor->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockWaveformEditor);
    mDockWaveformEditor->setFloating(true);
    mDockWaveformEditor->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockHistory);
    mDockHistory->setFloating(true);
    mDockHistory->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockInstruments);
    mDockInstruments->setFloating(true);
    mDockInstruments->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockWaveforms);
    mDockWaveforms->setFloating(true);
    mDockWaveforms->hide();
}

void MainWindow::settingsMessageBox(QMessageBox &msgbox) {
    auto settingsBtn = msgbox.addButton(tr("Change settings"), QMessageBox::ActionRole);
    msgbox.addButton(QMessageBox::Close);
    msgbox.setDefaultButton(settingsBtn);
    msgbox.exec();

    if (msgbox.clickedButton() == settingsBtn) {
        showConfigDialog();
    }
}

void MainWindow::updateWindowTitle() {
    setWindowTitle(QStringLiteral("%1[*] - Trackerboy").arg(mModuleFile.name()));
}

void MainWindow::updateOrderActions() {
    // bool canInsert = mOrderModel->canInsert();
    // mActionOrderInsert->setEnabled(canInsert);
    // mActionOrderDuplicate->setEnabled(canInsert);
    // mActionOrderRemove->setEnabled(mOrderModel->canRemove());
    // mActionOrderMoveUp->setEnabled(mOrderModel->canMoveUp());
    // mActionOrderMoveDown->setEnabled(mOrderModel->canMoveDown());
}

void MainWindow::setPlayingStatus(PlayingStatusText type) {
    static const char *PLAYING_STATUSES[] = {
        QT_TR_NOOP("Ready"),
        QT_TR_NOOP("Playing"),
        QT_TR_NOOP("Device error")
    };

    mStatusRenderer->setText(tr(PLAYING_STATUSES[(int)type]));
}

void MainWindow::disableMidi(bool causedByError) {
    mConfig.disableMidi();
    if (mConfigDialog) {
        mConfigDialog->resetControls();
    }

    if (!causedByError) {
        qCritical().noquote() << "[MIDI] Failed to initialize MIDI device:" << mMidi.lastErrorString();
    }

    if (isVisible()) {
        QMessageBox msgbox(this);
        msgbox.setIcon(QMessageBox::Critical);
        if (causedByError) {
            msgbox.setText(tr("MIDI device error"));
        } else {
            msgbox.setText(tr("Could not initialize MIDI device"));
        }
        msgbox.setInformativeText(mMidi.lastErrorString());
        settingsMessageBox(msgbox);
    }
}

void MainWindow::handleFocusChange(QWidget *oldWidget, QWidget *newWidget) {
    Q_UNUSED(oldWidget)

    // this handler is for determining where MIDI events will go, based on
    // who has focus. If MIDI is disabled we don't need to do anything

    // performance notes
    // searching is required by walking the newWidget's parents until either
    // nullptr or the editor dock widgets are found. Performance depends on
    // how deep newWidget is nested in the hierarchy.

    // alternative solution:
    // BaseEditor could have an eventFilter installed on all of its child widgets,
    // that check for FocusIn and FocusOut events, which we could emit a
    // signal for these and the MainWindow would handle them appropriately.

    if (mMidi.isEnabled()) {
        if (QApplication::activeModalWidget()) {
            return; // ignore if a dialog is open
        }

        // MIDI events by default go to the pattern editor
        IMidiReceiver *receiver = mPatternEditor;

        QWidget *widget = newWidget;
        while (widget) {
            // search if this widget's parent is the instrument or waveform editor dock
            // if it is, midi events will go to the editor's piano widget
            if (widget == mDockWaveformEditor) {
                receiver = static_cast<WaveEditor*>(mDockWaveformEditor->widget())->piano();
                break;
            } else if (widget == mDockInstrumentEditor) {
                receiver = static_cast<InstrumentEditor*>(mDockInstrumentEditor->widget())->piano();
                break;
            }
            widget = widget->parentWidget();
        }

        if (mMidiReceiver != receiver) {
            // change the receiver
            if (mMidiNoteDown) {
                // force the note off
                // if we don't do this, the previous receiver won't get the next noteOff message
                // and the note will be held indefinitely
                mMidiReceiver->midiNoteOff();
                mMidiNoteDown = false;
            }
            mMidiReceiver = receiver;

        }
    }

}

void MainWindow::openEditor(QDockWidget *editorDock, int item) {
    auto editor = qobject_cast<BaseEditor*>(editorDock->widget());
    if (editor) {
        editor->openItem(item);
        editorDock->show();
        editorDock->raise();
        editorDock->activateWindow();
    }
}

#undef TU
