#ifndef VICTORYHANDLER_H
#define VICTORYHANDLER_H

class Widget;
class HighlightPieces;

#include <QWidget>
#include <QDebug>
#include <QMessageBox>

class VictoryHandler : public QWidget
{
    Q_OBJECT

public:
    explicit VictoryHandler(QWidget *parent = nullptr);
    void checkGameOver();

private:
    Widget *widget;
    HighlightPieces *highlightPieces;
    bool isCheckmate();
    bool isStalemate();
    bool isDraw();
    bool isOwnPiece(int row, int col);
    bool isSquareAttacked(int row, int col, bool isWhite);
    void highlightMoves(int row, int col);

    QVector<QVector<char>> board;
    bool isWhiteTurn = false;
    bool isStarted = false;
    int stepsCount = 0;
    QSet<QPair<int, int>> possibleMoves;
};

#endif // VICTORYHANDLER_H
