
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "core/Trackerboy.hpp"
#include "core/Config.hpp"

#include "widgets/module/OrderEditor.hpp"
#include "widgets/PatternEditor.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QWidget>

class ModuleWindow : public QWidget {

    Q_OBJECT

public:

    explicit ModuleWindow(Trackerboy &app, ModuleDocument *doc, QWidget *parent = nullptr);

    ModuleDocument* document() noexcept;

    bool save();

    bool saveAs();

    static const char* MODULE_FILE_FILTER;

public slots:
    void applyConfiguration(Config::Categories categories);

protected:

    virtual void closeEvent(QCloseEvent *evt) override;

signals:
    void documentClosed(ModuleDocument *document);

private:

    void updateWindowTitle();

    bool maybeSave();

    //
    // Writes any retained data within the UI to the module
    //
    void commit();

    Trackerboy &mApp;
    ModuleDocument *mDocument;

    QHBoxLayout mLayout;
        OrderEditor mOrderWidget;
        PatternEditor mPatternEditor;




};