
#pragma once

#include <QFont>
#include <QPainter>

//
// Utility class for painting single characters in a grid of "cells". The size
// of a cell is determined by the given font. 
//
class CellPainter {

public:
    explicit CellPainter();

    int cellHeight() const;

    int cellWidth() const;

    void setFont(QFont const& font);

    //
    // Draws a cell at the given x and y coordinates. The x position of the
    // next cell is returned
    //
    int drawCell(QPainter &painter, char cell, int xpos, int ypos);

    int drawHex(QPainter &painter, int hex, int xpos, int ypos);

private:

    int mCellHeight;
    int mCellWidth;

    // 1-character string used by drawCell
    // this way we don't have to create a temporary QString every call
    // unnecessary if QString has small string optimization (don't think it does)
    QString mCellScratch;


};
