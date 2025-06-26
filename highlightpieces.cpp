#include "widget.h"
#include "highlightpieces.h"

HighlightPieces::HighlightPieces(QWidget *parent) {}

HighlightPieces::~HighlightPieces() {}

void HighlightPieces::highlightMoves(int row, int col)
{
    if (!isStarted) return;
    char piece = board[row][col];
    if (piece == ' ') return;

    possibleMoves.clear();

    // Determine possible moves based on the piece type
    switch (piece) {
    case 'R': case 'r': highlightRookMoves(row, col); break;
    case 'N': case 'n': highlightKnightMoves(row, col); break;
    case 'B': case 'b': highlightBishopMoves(row, col); break;
    case 'Q': case 'q': highlightQueenMoves(row, col); break;
    case 'K': case 'k': highlightKingMoves(row, col); break;
    case 'P': case 'p': highlightPawnMoves(row, col); break;
    }

    // Filter illegal moves (e.g., moves that expose the king to check)
    widget->filterIllegalMoves();
    widget->updatePieceCount();
}

void HighlightPieces::highlightPawnMoves(int row, int col)
{
    int direction = (board[row][col] == 'P') ? -1 : 1; // Fehérek felfelé, feketék lefelé

    // Egy mezőt előre léphet, ha az üres
    if (row + direction >= 0 && row + direction < 8 && board[row + direction][col] == ' ') {
        possibleMoves.insert({row + direction, col});

        // Ha az eredeti helyén van, akkor két mezőt is léphet előre, ha az is üres
        if ((board[row][col] == 'P' && row == 6) || (board[row][col] == 'p' && row == 1)) {
            if (board[row + 2 * direction][col] == ' ') {
                possibleMoves.insert({row + 2 * direction, col});
            }
        }
    }

    // Diagonális támadások (ellenfél bábuja esetén)
    if (col - 1 >= 0 && isEnemyPiece(row + direction, col - 1)) {
        possibleMoves.insert({row + direction, col - 1});
    }
    if (col + 1 < 8 && isEnemyPiece(row + direction, col + 1)) {
        possibleMoves.insert({row + direction, col + 1});
    }
}

void HighlightPieces::highlightRookMoves(int row, int col)
{
    // Horizontal and vertical moves
    for (int i = 1; i < 8; ++i) {
        if (row + i < 8 && (board[row + i][col] == ' ' || isEnemyPiece(row + i, col))) {
            possibleMoves.insert({row + i, col});
            if (board[row + i][col] != ' ') break; // Stop if an enemy piece is encountered
        }
        if (row - i >= 0 && (board[row - i][col] == ' ' || isEnemyPiece(row - i, col))) {
            possibleMoves.insert({row - i, col});
            if (board[row - i][col] != ' ') break;
        }
        if (col + i < 8 && (board[row][col + i] == ' ' || isEnemyPiece(row, col + i))) {
            possibleMoves.insert({row, col + i});
            if (board[row][col + i] != ' ') break;
        }
        if (col - i >= 0 && (board[row][col - i] == ' ' || isEnemyPiece(row, col - i))) {
            possibleMoves.insert({row, col - i});
            if (board[row][col - i] != ' ') break;
        }
    }
}

void HighlightPieces::highlightKnightMoves(int row, int col)
{
    QVector<QPair<int, int>> knightMoves = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1},
                                            {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
    for (const auto &move : knightMoves) {
        int newRow = row + move.first;
        int newCol = col + move.second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8 &&
            (board[newRow][newCol] == ' ' || isEnemyPiece(newRow, newCol))) {
            possibleMoves.insert({newRow, newCol});
        }
    }
}

void HighlightPieces::highlightBishopMoves(int row, int col)
{
    // Diagonal moves
    for (int i = 1; i < 8; ++i) {
        if (row + i < 8 && col + i < 8 && (board[row + i][col + i] == ' ' || isEnemyPiece(row + i, col + i))) {
            possibleMoves.insert({row + i, col + i});
            if (board[row + i][col + i] != ' ') break;
        }
        if (row + i < 8 && col - i >= 0 && (board[row + i][col - i] == ' ' || isEnemyPiece(row + i, col - i))) {
            possibleMoves.insert({row + i, col - i});
            if (board[row + i][col - i] != ' ') break;
        }
        if (row - i >= 0 && col + i < 8 && (board[row - i][col + i] == ' ' || isEnemyPiece(row - i, col + i))) {
            possibleMoves.insert({row - i, col + i});
            if (board[row - i][col + i] != ' ') break;
        }
        if (row - i >= 0 && col - i >= 0 && (board[row - i][col - i] == ' ' || isEnemyPiece(row - i, col - i))) {
            possibleMoves.insert({row - i, col - i});
            if (board[row - i][col - i] != ' ') break;
        }
    }
}

void HighlightPieces::highlightQueenMoves(int row, int col)
{
    highlightRookMoves(row, col);
    highlightBishopMoves(row, col);
}

void HighlightPieces::highlightKingMoves(int row, int col)
{
    QVector<QPair<int, int>> kingMoves = {{1, 0}, {-1, 0}, {0, 1}, {0, -1},
                                          {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto &move : kingMoves) {
        int newRow = row + move.first;
        int newCol = col + move.second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8 &&
            (board[newRow][newCol] == ' ' || isEnemyPiece(newRow, newCol))) {
            possibleMoves.insert({newRow, newCol});
        }
    }
    highlightCastling(row, col);
}

void HighlightPieces::highlightCastling(int row, int col)
{
    if (hasMoved[row][col]) return;
    if (!hasMoved[row][0] && board[row][1] == ' ' && board[row][2] == ' ' && board[row][3] == ' ')
        possibleMoves.insert({row, 2});
    if (!hasMoved[row][7] && board[row][5] == ' ' && board[row][6] == ' ')
        possibleMoves.insert({row, 6});
}

bool HighlightPieces::isEnemyPiece(int row, int col)
{
    char piece = board[row][col];
    if (piece == ' ' || (isWhiteTurn && piece >= 'A' && piece <= 'Z') ||
        (!isWhiteTurn && piece >= 'a' && piece <= 'z')) {
        return false; // Ha a mező üres vagy a saját színű bábu, nem támadható
    }
    return true; // Ellenfél bábujának támadása
}
