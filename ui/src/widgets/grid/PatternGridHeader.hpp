
#pragma once

#include "core/model/PatternModel.hpp"
#include "core/graphics/PatternLayout.hpp"
#include "core/ChannelOutput.hpp"
#include "core/Palette.hpp"

#include <QPen>
#include <QWidget>

class PatternGridHeader : public QWidget {

    Q_OBJECT

public:

    PatternGridHeader(PatternModel &model, QWidget *parent = nullptr);

    void setColors(Palette const& colorTable);

    void setPatternLayout(PatternLayout const* layout);

    void toggleTrack(int track);

    void soloTrack(int track);

    void unmuteAll();

signals:
    void outputChanged(ChannelOutput::Flags flags);

protected:

    virtual void contextMenuEvent(QContextMenuEvent *evt) override;

    virtual void paintEvent(QPaintEvent *evt) override;

    virtual void leaveEvent(QEvent *evt) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *evt) override;

    virtual void mouseMoveEvent(QMouseEvent *evt) override;

    virtual void mousePressEvent(QMouseEvent *evt) override;


private:
    Q_DISABLE_COPY(PatternGridHeader)

    enum class HoverKind {
        track,          // mouse is over the track
        addEffect,      // mouse is over the add effect (plus) button
        removeEffect    // mouse is over the remove effect (minus) button
    };

    static constexpr int HEIGHT = 36;
    static constexpr int HOVER_NONE = -1;

    void setOutputFlags(ChannelOutput::Flags flags);

    void setTrackHover(int hover);

    void setHoverKind(HoverKind kind);

    PatternModel &mModel;
    PatternLayout const* mLayout;

    // header stuff
    int mTrackHover;
    HoverKind mHoverKind;

    ChannelOutput::Flags mTrackFlags;

    QColor mColorBackground;
    QColor mColorForeground1;
    QColor mColorForeground2;

    QColor mColorEnabled;
    QColor mColorDisabled;

    QPen mLinePen;

};
