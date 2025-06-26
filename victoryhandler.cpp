#include "widget.h"
#include "victoryhandler.h"
#include "highlightpieces.h"
#include <QMessageBox>

VictoryHandler::VictoryHandler(QWidget *parent) {}

void VictoryHandler::checkGameOver()
{
    if (isCheckmate()) {
        qDebug() << "🏁 Checkmate észlelve!";
        QMessageBox::information(this, "Játék vége", isWhiteTurn ? "Fekete nyert (matt)!" : "Fehér nyert (matt)!");
        isStarted = false;
    }
    else if (isStalemate()) {
        qDebug() << "⚖️ Stalemate észlelve!";
        QMessageBox::information(this, "Játék vége", "Döntetlen (pat)!");
        isStarted = false;
    }
    else if (isDraw()) {
        qDebug() << "🤝 Döntetlen észlelve!";
        QMessageBox::information(this, "Játék vége", "Döntetlen!");
        isStarted = false;
    }
    possibleMoves.clear();
}

bool VictoryHandler::isOwnPiece(int row, int col)
{
    if (board[row][col] == ' ') return false; // Üres mező

    char piece = board[row][col];

    if (isWhiteTurn) {
        // Fehér játékos: a saját bábuk nagybetűsek
        return (piece >= 'A' && piece <= 'Z');
    } else {
        // Fekete játékos: a saját bábuk kisbetűsek
        return (piece >= 'a' && piece <= 'z');
    }
}

bool VictoryHandler::isCheckmate()
{
    // Find the king's position
    int kingRow = -1, kingCol = -1;
    char kingChar = isWhiteTurn ? 'K' : 'k'; // Fehér király: 'K', Fekete király: 'k'

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board[row][col] == kingChar) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }

    // Ha nem találjuk a királyt (valami nagyobb hiba történt)
    if (kingRow == -1 || kingCol == -1) return false;

    // Ellenőrizzük, hogy sakkban van-e
    if (!widget->isSquareAttacked(kingRow, kingCol, isWhiteTurn)) return false;

    // Ha sakkban van, ellenőrizzük, hogy van-e érvényes lépése
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (widget->isOwnPiece(row, col)) {  // Ha saját bábunk
                highlightPieces->highlightMoves(row, col);
                if (!possibleMoves.isEmpty()) return false;  // Ha van lépés, nem matt
            }
        }
    }

    return true; // Nincs érvényes lépés → matt
}

bool VictoryHandler::isStalemate()
{
    // Find the king's position
    int kingRow = -1, kingCol = -1;
    char kingChar = isWhiteTurn ? 'K' : 'k'; // Fehér király: 'K', Fekete király: 'k'

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board[row][col] == kingChar) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
    }

    // Ha nem találjuk a királyt (valami nagyobb hiba történt)
    if (kingRow == -1 || kingCol == -1) return false;

    // Ha a király támadás alatt áll, nem lehet patt
    if (widget->isSquareAttacked(kingRow, kingCol, isWhiteTurn)) return false;

    // Ha nincs érvényes lépés, patt
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (isOwnPiece(row, col)) {  // Ha saját bábunk
                highlightPieces->highlightMoves(row, col);
                if (!possibleMoves.isEmpty()) return false;  // Ha van lépés, nem patt
            }
        }
    }

    return true; // Nincs érvényes lépés, de a király nincs sakkban → patt
}

bool VictoryHandler::isDraw()
{
    // Anyaghiány esetén döntetlen (pl. csak két király maradt a táblán)
    int pieceCount = 0;
    for (const auto &row : board) {
        for (char piece : row) {
            if (piece != ' ') pieceCount++;
        }
    }
    if (pieceCount <= 3) return true; // Király vs király vagy király + kisebb figura

    if (stepsCount >= 100) { // 100 fél lépés = 50 teljes lépés
        qDebug() << "📜 Ötven lépés szabály miatt döntetlen!";
        return true;
    }

    return false;
}
