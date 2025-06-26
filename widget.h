#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QVector>
#include <QProcess>

class UCIEngine;
class HighlightPieces;
class VictoryHandler;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void initializeBoard();
    void highlightMoves(int row, int col);
    void makeMove(const QString &move);
    void highlightPawnMoves(int row, int col);
    void highlightRookMoves(int row, int col);
    void highlightKnightMoves(int row, int col);
    void highlightBishopMoves(int row, int col);
    void highlightQueenMoves(int row, int col);
    void highlightKingMoves(int row, int col);
    void highlightCastling(int row, int col);
    bool isEnPassant(int row, int col);
    void filterIllegalMoves();
    bool doesMoveExposeKing(QPair<int, int> move);
    bool isSquareAttacked(int row, int col, bool isWhite);
    bool applyMove(QString move);
    bool isEnemyPiece(int row, int col);
    void onBestMoveReceived(QString bestMove);
    bool isValidMove(QString move);
    void updatePossibleMoves();
    QVector<QPair<int, int>> getLegalMoves(int row, int col);
    void addPawnMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite);
    void addRookMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite);
    void addKnightMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite);
    void addBishopMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite);
    void addQueenMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite);
    void addKingMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite);
    QPair<QPair<int, int>, QPair<int, int>> convertUciToCoords(QString uciMove);
    void startNewGame();
    void resetGame();
    void checkGameOver();
    bool isCheckmate();
    bool isStalemate();
    bool isDraw();
    bool isOwnPiece(int row, int col);
    void updatePieceCount();
    void clearPieceCount();
    void clearStepsCounter();
    bool findKingPosition(int &kingRow, int &kingCol);
    QSet<QPair<int, int>> possibleMoves;
    QVector<QVector<bool>> hasMoved;
    QPair<int, int> enPassantTarget;
    int selectedRow = -1;
    int selectedCol = -1;
    bool isWhiteTurn = true;
    QStringList moveHistory;
    QMap<QString, QString> boardMap;
    int stepsCount = 0;
    bool isStarted = false;

private:
    Ui::Widget *ui;
    UCIEngine *uciEngine;
    HighlightPieces *highlightPieces;
    VictoryHandler *victoryHandler;
    QVector<QVector<char>> board;

signals:
    void moveMade(QString move);
};

#endif // WIDGET_H
