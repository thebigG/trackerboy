
#pragma once

#include "core/graphics/CellPainter.hpp"
#include "core/graphics/PatternLayout.hpp"
#include "core/Palette.hpp"
#include "core/PatternCursor.hpp"
#include "core/PatternSelection.hpp"

#include "trackerboy/data/Pattern.hpp"

#include <QColor>
#include <QPen>
#include <QFont>
#include <QPainter>
#include <QWidget>

#include <array>


class PatternPainter : public CellPainter {

public:

    enum RowType {
        RowCurrent,
        RowEdit,
        RowPlayer
    };

    PatternPainter(QFont const& font);

    //int rownoWidth() const;

    //int trackWidth() const;

    int patternStartPos() const;

    //
    // Returns true if accidentals will be drawn using flats instead of sharps
    //
    bool flats() const;

    void setColors(Palette const& colors);

    void setFirstHighlight(int interval);

    void setSecondHighlight(int interval);

    void setFlats(bool flats);

    // drawing functions

    //
    // draws the background for pattern data.
    //
    void drawBackground(QPainter &p, PatternLayout const& l, int ypos, int rowStart, int rows) const;


    //
    // Draws the cursor row background or player row background
    //
    void drawRowBackground(QPainter &p, PatternLayout const& l, RowType type, int row) const;

    void drawCursor(QPainter &p, PatternLayout const& l, PatternCursor cursor) const;

    void drawLines(QPainter &p, PatternLayout const& l, int height) const;

    //
    // Draws pattern data from the given pattern and range of rows starting
    // at the given y position. The y position of the next row is returned
    //
    int drawPattern(
        QPainter &p, PatternLayout const& l,
        trackerboy::Pattern const& pattern,
        int rowStart,
        int rowEnd,
        int ypos
    );

    //
    // Draws the selection rectangle
    //
    void drawSelection(QPainter &painter, QRect const& rect) const;

    //
    // Paints "nothing" or "-" for the cell(s). The x position of the next
    // cell is returned
    //
    int drawNone(QPainter &painter, int cells, int xpos, int ypos);

    //
    // Paints a note at the given x and y position. The x position of the next
    // cell is returned.
    //
    int drawNote(QPainter &painter, uint8_t note, int xpos, int ypos);

private:

    using NoteTable = std::array<char, 24>;

    static NoteTable const NOTE_TABLE_FLATS;
    static NoteTable const NOTE_TABLE_SHARPS;

    //
    // Returns a reference to the cached QPen after setting its color to the
    // given parameter. Use this function instead of creating a temporary QPen
    //
    QPen const& pen(QColor const& color) const;

    int highlightIndex(int rowno) const;
    
    int mHighlightInterval1;
    int mHighlightInterval2;

    NoteTable const *mNoteTable;

    std::array<QColor, 3> mForegroundColors;
    std::array<QColor, 3> mBackgroundColors;

    QColor mColorInstrument;
    QColor mColorEffect;

    QColor mColorSelection;
    QColor mColorCursor;
    QColor mColorLine;

    std::array<QColor, 3> mRowColors;

    // pen used in all draw functions, reusing this one prevents the
    // need to create a temporary
    mutable QPen mPen;


};
