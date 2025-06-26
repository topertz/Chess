#ifndef HIGHLIGHTPIECES_H
#define HIGHLIGHTPIECES_H

class Widget;

#include <QWidget>
#include <QSet>
#include <QPair>
#include <QVector>

class HighlightPieces : public QWidget
{
    Q_OBJECT

public:
    explicit HighlightPieces(QWidget *parent = nullptr);
    ~HighlightPieces();
    void highlightMoves(int row, int col);
    void highlightPawnMoves(int row, int col);
    void highlightRookMoves(int row, int col);
    void highlightKnightMoves(int row, int col);
    void highlightBishopMoves(int row, int col);
    void highlightQueenMoves(int row, int col);
    void highlightKingMoves(int row, int col);

private:
    Widget *widget;
    void highlightCastling(int row, int col);
    void filterIllegalMoves();
    void updatePieceCount();
    bool isEnemyPiece(int row, int col);

    int selectedRow = -1;
    int selectedCol = -1;
    bool isStarted = false;
    QVector<QVector<char>> board;
    QVector<QVector<bool>> hasMoved;
    QSet<QPair<int, int>> possibleMoves;
    bool isWhiteTurn = true;
    int stepsCount = 0;
};

#endif // HIGHLIGHTPIECES_H
