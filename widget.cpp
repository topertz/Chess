#include "widget.h"
#include "ui_widget.h"
#include "uciengine.h"
#include "highlightpieces.h"
#include "victoryhandler.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QSet>
#include <QTimer>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , uciEngine(new UCIEngine(this))
    , highlightPieces(new HighlightPieces(this))
    , victoryHandler(new VictoryHandler(this))
{
    ui->setupUi(this);
    setWindowTitle("Chess");
    resize(800, 610);
    initializeBoard();
    uciEngine->startEngine();
    connect(ui->newgameButton, &QPushButton::clicked, this, &Widget::startNewGame);
    connect(ui->resetgameButton, &QPushButton::clicked, this, &Widget::resetGame);
    connect(uciEngine, &UCIEngine::bestMoveFound, this, &Widget::onBestMoveReceived);
    connect(uciEngine->uciProcess, &QProcess::readyReadStandardOutput, uciEngine, &UCIEngine::handleEngineOutput);
    connect(uciEngine->uciProcess, &QProcess::readyReadStandardError, uciEngine, &UCIEngine::handleEngineErrorOutput);
    connect(uciEngine->uciProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            uciEngine, &UCIEngine::handleProcessFinished);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    if(!isStarted) return;
    QPainter painter(this);
    int boardSize = 600;  // 🔹 Fix méret a sakktáblának
    int squareSize = boardSize / 8;

    // Sakktábla rajzolása
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QRect square(col * squareSize, row * squareSize, squareSize, squareSize);
            painter.setBrush((row + col) % 2 == 0 ? Qt::white : Qt::gray);
            painter.drawRect(square);
        }
    }

    // Betűk és számok rajzolása (A-H, 1-8)
    QFont font("Arial", 14, QFont::Bold);
    painter.setFont(font);
    painter.setPen(Qt::black);

    for (int i = 0; i < 8; ++i) {
        painter.drawText(5, (i + 0.7) * squareSize, QString::number(8 - i));  // Sorok (1-8)
        painter.drawText((i + 0.4) * squareSize, boardSize - 2.5, QChar('A' + i));  // Oszlopok (A-H)
    }

    // Unicode karakterek a bábukhoz
    QMap<char, QString> pieceMap = {
        {'K', "\u2654"}, {'Q', "\u2655"}, {'R', "\u2656"}, {'B', "\u2657"},
        {'N', "\u2658"}, {'P', "\u2659"}, {'k', "\u265A"}, {'q', "\u265B"},
        {'r', "\u265C"}, {'b', "\u265D"}, {'n', "\u265E"}, {'p', "\u265F"}
    };

    // Bábuk rajzolása középre igazítva
    QFont pieceFont("Arial", 36);
    painter.setFont(pieceFont);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            char piece = board[row][col];
            if (piece != ' ') {
                QRect textRect(col * squareSize, row * squareSize, squareSize, squareSize);
                painter.drawText(textRect, Qt::AlignCenter, pieceMap[piece]);
            }
        }
    }

    // Lehetséges lépések kiemelése
    painter.setBrush(QColor(255, 255, 0, 100));  // Átlátszó sárga szín
    for (auto move : possibleMoves) {
        int moveRow = move.first;
        int moveCol = move.second;
        painter.drawRect(moveCol * squareSize, moveRow * squareSize, squareSize, squareSize);
    }
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    // A négyzetek tényleges méretének kiszámítása
    int squareSize = qMin(width(), height()) / 8;

    // A kattintás sor és oszlop koordinátái
    int col = event->position().x() / squareSize;
    int row = event->position().y() / squareSize;

    // Ellenőrizzük, hogy a kattintás a táblán belül van-e (0-7 sorok és oszlopok)
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return;
    }
    // Ha nincs még kiválasztott bábu
    if (selectedRow == -1 && selectedCol == -1) {
        char piece = board[row][col];

        // Ellenőrizzük, hogy a kattintott bábu a megfelelő színű-e
        if (piece != ' ' && ((isWhiteTurn && piece >= 'A' && piece <= 'Z') ||
                             (!isWhiteTurn && piece >= 'a' && piece <= 'z'))) {
            selectedRow = row;
            selectedCol = col;
            highlightMoves(row, col);
            update();
        }
    }
    else {  // Ha már van kiválasztott bábu
        if (possibleMoves.contains(QPair<int, int>(row, col))) {
            // Alkalmazzuk a lépést
            QString move = QString("%1%2%3%4")
                               .arg(QChar('a' + selectedCol))
                               .arg(8 - selectedRow)
                               .arg(QChar('a' + col))
                               .arg(8 - row);

            bool moveSuccess = applyMove(move);  // Megnézzük, hogy sikerült-e a lépés

            if (moveSuccess) {
                // Csak akkor töröljük a kiválasztást és a lehetséges lépéseket, ha a lépés sikeres volt
                selectedRow = -1;
                selectedCol = -1;
                possibleMoves.clear();
                stepsCount++;
                ui->stepLabel->setText(QString("Steps Count: %1").arg(stepsCount));
                update();
            }
            else {
                qDebug() << "Move failed, selection remains.";
            }
        }
        else {  // Ha a kattintás nem érvényes lépés volt, csak töröljük a kijelölést
            selectedRow = -1;
            selectedCol = -1;
            possibleMoves.clear();
            update();
        }
    }
}

void Widget::initializeBoard()
{
    board = QVector<QVector<char>>(8, QVector<char>(8, ' '));
    hasMoved = QVector<QVector<bool>>(8, QVector<bool>(8, false));
    enPassantTarget = QPair<int, int>(-1, -1);

    // Kezdő pozíció beállítása
    QString initialPositionStr = "rnbqkbnrpppppppp                                PPPPPPPPRNBQKBNR";
    QByteArray initialPosition = initialPositionStr.toLatin1();

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            board[row][col] = initialPosition.at(row * 8 + col);
        }
    }

    isWhiteTurn = true;  // A fehér kezd
}

void Widget::updatePieceCount()
{
    int whiteCount = 0, blackCount = 0;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            char piece = board[row][col];
            if (piece == ' ') continue;

            if (piece >= 'A' && piece <= 'Z') {
                whiteCount++;
            } else if (piece >= 'a' && piece <= 'z') {
                blackCount++;
            }
        }
    }

    int whiteCaptured = 16 - whiteCount;
    int blackCaptured = 16 - blackCount;

    ui->whitePiecesLabel->setText(QString("White Pieces: %1").arg(whiteCount));
    ui->blackPiecesLabel->setText(QString("Black Pieces: %1").arg(blackCount));
    ui->whiteCapturedLabel->setText(QString("White Pieces Knocked Out: %1").arg(whiteCaptured));
    ui->blackCapturedLabel->setText(QString("Black Pieces Knocked Out: %1").arg(blackCaptured));
}

void Widget::highlightMoves(int row, int col)
{
    if (!isStarted) return;
    char piece = board[row][col];
    if (piece == ' ') return;

    possibleMoves.clear();

    switch (piece) {
    case 'R': case 'r': highlightRookMoves(row, col); break;
    case 'N': case 'n': highlightKnightMoves(row, col); break;
    case 'B': case 'b': highlightBishopMoves(row, col); break;
    case 'Q': case 'q': highlightQueenMoves(row, col); break;
    case 'K': case 'k': highlightKingMoves(row, col); break;
    case 'P': case 'p': highlightPawnMoves(row, col); break;
    }

    filterIllegalMoves(); // Király sakkellenőrzése
    updatePieceCount();
}

void Widget::highlightPawnMoves(int row, int col)
{
    int direction = (board[row][col] == 'P') ? -1 : 1;
    if (row + direction >= 0 && row + direction < 8 && board[row + direction][col] == ' ') {
        possibleMoves.insert({row + direction, col});
        if ((board[row][col] == 'P' && row == 6) || (board[row][col] == 'p' && row == 1)) {
            if (board[row + 2 * direction][col] == ' ') {
                possibleMoves.insert({row + 2 * direction, col});
            }
        }
    }
    if (row + direction >= 0 && row + direction < 8) {
        if (col - 1 >= 0 && isEnemyPiece(row + direction, col - 1)) {
            possibleMoves.insert({row + direction, col - 1});
        }
        if (col + 1 < 8 && isEnemyPiece(row + direction, col + 1)) {
            possibleMoves.insert({row + direction, col + 1});
        }
        if (col - 1 >= 0 && isEnPassant(row + direction, col - 1) && board[row][col - 1] != ' ') {
            possibleMoves.insert({row + direction, col - 1});
        }
        if (col + 1 < 8 && isEnPassant(row + direction, col + 1) && board[row][col + 1] != ' ') {
            possibleMoves.insert({row + direction, col + 1});
        }
    }
}

void Widget::highlightRookMoves(int row, int col)
{
    QVector<QPair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto& dir : directions) {
        int newRow = row + dir.first, newCol = col + dir.second;
        while (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            if (board[newRow][newCol] == ' ') {
                possibleMoves.insert({newRow, newCol});
            } else {
                if (isEnemyPiece(newRow, newCol)) possibleMoves.insert({newRow, newCol});
                break; // Ha saját vagy ellenséges bábu van, megállunk
            }
            newRow += dir.first;
            newCol += dir.second;
        }
    }
}

void Widget::highlightKnightMoves(int row, int col)
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

void Widget::highlightBishopMoves(int row, int col)
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

void Widget::highlightQueenMoves(int row, int col)
{
    highlightRookMoves(row, col);
    highlightBishopMoves(row, col);
}

void Widget::highlightKingMoves(int row, int col)
{
    QVector<QPair<int, int>> kingMoves = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto &move : kingMoves) {
        int newRow = row + move.first;
        int newCol = col + move.second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8 &&
            (board[newRow][newCol] == ' ' || isEnemyPiece(newRow, newCol))) {
            if (!doesMoveExposeKing({newRow, newCol})) { // ELLENŐRZI, HOGY SAKKBAN LENNE-E
                possibleMoves.insert({newRow, newCol});
            }
        }
    }
    highlightCastling(row, col);
}

void Widget::highlightCastling(int row, int col)
{
    if (hasMoved[row][col]) return;

    // Hosszú sánc (balra)
    if (!hasMoved[row][0] && board[row][1] == ' ' && board[row][2] == ' ' && board[row][3] == ' ') {
        if (!doesMoveExposeKing({row, 2}) && !doesMoveExposeKing({row, 3})) { // Sakkellenőrzés az áthaladó mezőkön
            possibleMoves.insert({row, 2});
        }
    }

    // Rövid sánc (jobbra)
    if (!hasMoved[row][7] && board[row][5] == ' ' && board[row][6] == ' ') {
        if (!doesMoveExposeKing({row, 5}) && !doesMoveExposeKing({row, 6})) { // Sakkellenőrzés az áthaladó mezőkön
            possibleMoves.insert({row, 6});
        }
    }
}

void Widget::filterIllegalMoves()
{
    QSet<QPair<int, int>> validMoves;
    for (auto move : possibleMoves) {
        if (!doesMoveExposeKing(move)) validMoves.insert(move);
    }
    possibleMoves = validMoves;
}

bool Widget::doesMoveExposeKing(QPair<int, int> move)
{
    if (selectedRow < 0 || selectedRow >= 8 || selectedCol < 0 || selectedCol >= 8) {
        return false;
    }

    int fromRow = selectedRow;
    int fromCol = selectedCol;
    int toRow = move.first;
    int toCol = move.second;

    char piece = board[fromRow][fromCol];
    if (piece == ' ') {
        qDebug() << "HIBA: Nincs bábu a kiválasztott mezőn!";
        return false;
    }

    char capturedPiece = board[toRow][toCol];

    // Ideiglenesen végrehajtjuk a lépést
    board[fromRow][fromCol] = ' ';
    board[toRow][toCol] = piece;

    // Megkeressük a saját királyunk pozícióját
    int kingRow = -1, kingCol = -1;
    char kingChar = (piece >= 'A' && piece <= 'Z') ? 'K' : 'k';

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board[row][col] == kingChar) {
                kingRow = row;
                kingCol = col;
            }
        }
    }

    if (kingRow == -1 || kingCol == -1) {
        qDebug() << "HIBA: Nem található a király!";
        return false;
    }

    // Ha a lépés maga a király lépése volt
    if (piece == 'K' || piece == 'k') {
        kingRow = toRow;
        kingCol = toCol;
    }

    // Ellenőrizzük, hogy az ellenfél sakkban tartja-e a királyt
    bool inCheck = isSquareAttacked(kingRow, kingCol, piece >= 'A' && piece <= 'Z');

    // Visszaállítjuk az eredeti állapotot
    board[fromRow][fromCol] = piece;
    board[toRow][toCol] = capturedPiece;

    return inCheck;
}

bool Widget::isSquareAttacked(int row, int col, bool isWhite)
{
    char enemyPawn = isWhite ? 'p' : 'P';
    char enemyRook = isWhite ? 'r' : 'R';
    char enemyKnight = isWhite ? 'n' : 'N';
    char enemyBishop = isWhite ? 'b' : 'B';
    char enemyQueen = isWhite ? 'q' : 'Q';
    char enemyKing = isWhite ? 'k' : 'K';

    // Pawn attacks
    int direction = isWhite ? -1 : 1;
    if (row + direction >= 0 && row + direction < 8) {
        if (col - 1 >= 0 && board[row + direction][col - 1] == enemyPawn) return true;
        if (col + 1 < 8 && board[row + direction][col + 1] == enemyPawn) return true;
    }

    // Knight attacks
    QVector<QPair<int, int>> knightMoves = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
    for (const auto &move : knightMoves) {
        int newRow = row + move.first;
        int newCol = col + move.second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8 && board[newRow][newCol] == enemyKnight) {
            return true;
        }
    }

    // Rook and Queen (horizontal and vertical)
    QVector<QPair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto &dir : directions) {
        int newRow = row, newCol = col;
        while (true) {
            newRow += dir.first;
            newCol += dir.second;
            if (newRow < 0 || newRow >= 8 || newCol < 0 || newCol >= 8) break;
            if (board[newRow][newCol] == ' ') continue;
            if (board[newRow][newCol] == enemyRook || board[newRow][newCol] == enemyQueen) return true;
            break;
        }
    }

    // Bishop and Queen (diagonal)
    QVector<QPair<int, int>> diagonalDirections = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto &dir : diagonalDirections) {
        int newRow = row, newCol = col;
        while (true) {
            newRow += dir.first;
            newCol += dir.second;
            if (newRow < 0 || newRow >= 8 || newCol < 0 || newCol >= 8) break;
            if (board[newRow][newCol] == ' ') continue;
            if (board[newRow][newCol] == enemyBishop || board[newRow][newCol] == enemyQueen) return true;
            break;
        }
    }

    // King attacks
    QVector<QPair<int, int>> kingMoves = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto &move : kingMoves) {
        int newRow = row + move.first;
        int newCol = col + move.second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8 && board[newRow][newCol] == enemyKing) {
            return true;
        }
    }

    return false;
}

bool Widget::isEnPassant(int row, int col)
{
    if (enPassantTarget != QPair<int, int>(row, col)) {
        return false; // Az en passant célmező nem egyezik
    }

    // Az éppen lépő bábu gyalog kell legyen
    char piece = board[selectedRow][selectedCol];
    if (piece != 'P' && piece != 'p') {
        return false; // Csak gyalog hajthat végre en passant ütést
    }

    // Az en passant célmezőnek egy üres mezőnek kell lennie
    if (board[row][col] != ' ') {
        return false;
    }

    // Az ellenséges gyalognak közvetlenül mellette kell lennie
    int enemyRow = (piece == 'P') ? row + 1 : row - 1;
    if (enemyRow >= 0 && enemyRow < 8) {
        char enemyPawn = board[enemyRow][col];
        if ((piece == 'P' && enemyPawn == 'p') || (piece == 'p' && enemyPawn == 'P')) {
            return true; // Az en passant szabály érvényes
        }
    }

    return false;
}

void Widget::checkGameOver()
{
    if (isCheckmate()) {
        qDebug() << "🏁 Checkmate észlelve!";
        QMessageBox::information(this, "Játék vége", isWhiteTurn ? "Fekete nyert (matt)!" : "Fehér nyert (matt)!");
        isStarted = false;
        return; // Nincs további ellenőrzés szükséges
    }
    if (isStalemate()) {
        qDebug() << "⚖️ Stalemate észlelve!";
        QMessageBox::information(this, "Játék vége", "Döntetlen (pat)!");
        isStarted = false;
        return;
    }
    if (isDraw()) {
        qDebug() << "🤝 Döntetlen észlelve!";
        QMessageBox::information(this, "Játék vége", "Döntetlen (anyaghiány vagy 50 lépés szabály)!");
        isStarted = false;
        return;
    }
    possibleMoves.clear(); // Minden esetben töröljük az előző lépéseket
}

bool Widget::isOwnPiece(int row, int col)
{
    if (row < 0 || row >= 8 || col < 0 || col >= 8 || board[row][col] == ' ') return false; // Határon kívüli vagy üres mező

    return isWhiteTurn ? std::isupper(board[row][col]) : std::islower(board[row][col]);
}

bool Widget::findKingPosition(int &kingRow, int &kingCol) {
    char kingChar = isWhiteTurn ? 'K' : 'k';

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board[row][col] == kingChar) {
                kingRow = row;
                kingCol = col;
                return true;
            }
        }
    }

    qDebug() << "HIBA: Király nem található a táblán!";
    return false;
}

bool Widget::isCheckmate()
{
    int kingRow, kingCol;
    if (!findKingPosition(kingRow, kingCol)) return false; // Biztonsági ellenőrzés

    if (!isSquareAttacked(kingRow, kingCol, isWhiteTurn)) return false; // Ha nincs sakk, nincs matt

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (isOwnPiece(row, col)) {
                highlightMoves(row, col);
                if (!possibleMoves.isEmpty()) return false; // Van érvényes lépés → nem matt
            }
        }
    }

    return true; // Ha nincs érvényes lépés → matt
}

bool Widget::isStalemate()
{
    int kingRow, kingCol;
    if (!findKingPosition(kingRow, kingCol)) return false; // Biztonsági ellenőrzés

    if (isSquareAttacked(kingRow, kingCol, isWhiteTurn)) return false; // Ha sakkban van, nem patt

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (isOwnPiece(row, col)) {
                highlightMoves(row, col);
                if (!possibleMoves.isEmpty()) return false; // Van lépés → nem patt
            }
        }
    }

    return true; // Ha nincs érvényes lépés, de a király nincs sakkban → patt
}

bool Widget::isDraw()
{
    int pieceCount = 0;
    int bishopCount = 0;
    int knightCount = 0;
    bool hasOtherPiece = false;
    std::vector<int> bishopColors; // Futók mezőszínének tárolása

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            char piece = board[row][col];
            if (piece != ' ') {
                pieceCount++;

                if (piece == 'B' || piece == 'b') {
                    bishopCount++;
                    bishopColors.push_back((row + col) % 2); // 0 = világos mező, 1 = sötét mező
                }
                if (piece == 'N' || piece == 'n') knightCount++;

                if (piece != 'K' && piece != 'k' && piece != 'B' && piece != 'b' && piece != 'N' && piece != 'n') {
                    hasOtherPiece = true; // Ha van más figura, akkor nincs anyaghiányos döntetlen
                }
            }
        }
    }

    if (hasOtherPiece) return false; // Ha más figura van, nem döntetlen

    // Anyaghiány ellenőrzés
    if (pieceCount == 2) return true; // Csak két király → döntetlen
    if (pieceCount == 3 && (bishopCount == 1 || knightCount == 1)) return true; // Király vs. Király+Huszár/Futó

    if (bishopCount == 2 && bishopColors.size() == 2) {
        if (bishopColors[0] == bishopColors[1]) return true; // Két futó azonos mezőszínen → döntetlen
    }

    // Ötven lépés szabály ellenőrzése
    if (stepsCount >= 100) {
        qDebug() << "📜 Ötven lépés szabály miatt döntetlen!";
        return true;
    }

    return false;
}

void Widget::onBestMoveReceived(QString bestMove)
{
    if (bestMove.isEmpty() || bestMove == "(none)") {
        qDebug() << "⚠️ No best move received from engine!";
        return;
    }

    qDebug() << "?? Stockfish best move: " << bestMove;
    qDebug() << "? Current turn before applying move: " << (isWhiteTurn ? "White" : "Black");

    if (isWhiteTurn) {
        qDebug() << "⚠️ Ignoring engine move, it's White's turn!";
        return;
    }

    // ⛔ **HIBA MEGOLDÁSA:** Ellenőrizzük, hogy a lépés valóban végrehajtható-e!
    if (!applyMove(bestMove)) {
        qDebug() << "❌ Move application failed!";
        return;
    }

    checkGameOver();
}

void Widget::addPawnMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite) {
    highlightPawnMoves(row, col);
    moves = possibleMoves.values().toVector();  // Átalakítás QVector-ba
}

void Widget::addRookMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite) {
    highlightRookMoves(row, col);
    moves = possibleMoves.values().toVector();
}

void Widget::addKnightMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite) {
    highlightKnightMoves(row, col);
    moves = possibleMoves.values().toVector();
}

void Widget::addBishopMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite) {
    highlightBishopMoves(row, col);
    moves = possibleMoves.values().toVector();
}

void Widget::addQueenMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite) {
    highlightQueenMoves(row, col);
    moves = possibleMoves.values().toVector();
}

void Widget::addKingMoves(QVector<QPair<int, int>>& moves, int row, int col, bool isWhite) {
    highlightKingMoves(row, col);
    moves = possibleMoves.values().toVector();
}

QPair<QPair<int, int>, QPair<int, int>> Widget::convertUciToCoords(QString uciMove) {
    if (uciMove.length() != 4) {
        qDebug() << "❌ Invalid UCI move format:" << uciMove;
        return { {-1, -1}, {-1, -1} }; // Hibás koordináta
    }

    return {
        { 8 - uciMove[1].digitValue(), uciMove[0].toLatin1() - 'a' },
        { 8 - uciMove[3].digitValue(), uciMove[2].toLatin1() - 'a' }
    };
}

QVector<QPair<int, int>> Widget::getLegalMoves(int row, int col) {
    possibleMoves.clear();
    QChar piece = board[row][col];

    if (piece == ' ') {
        qDebug() << "⚠️ No piece at (" << row << "," << col << ")";
        return {};
    }
    bool pieceIsWhite = piece.isUpper();

    // Ensure we only get legal moves for the correct side
    if ((isWhiteTurn && !pieceIsWhite) || (!isWhiteTurn && pieceIsWhite)) {
        qDebug() << "⚠️ It's not this piece's turn!";
        return {};
    }

    switch (piece.toLower().toLatin1()) {
    case 'p': highlightPawnMoves(row, col); break;
    case 'r': highlightRookMoves(row, col); break;
    case 'n': highlightKnightMoves(row, col); break;
    case 'b': highlightBishopMoves(row, col); break;
    case 'q': highlightQueenMoves(row, col); break;
    case 'k': highlightKingMoves(row, col); break;
    }
    filterIllegalMoves();
    return possibleMoves.values().toVector();
}

void Widget::updatePossibleMoves()
{
    possibleMoves.clear();

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QChar piece = board[row][col];

            if (piece != ' ') {
                bool pieceIsWhite = piece.isUpper();

                if ((isWhiteTurn && pieceIsWhite) || (!isWhiteTurn && !pieceIsWhite)) {
                    QVector<QPair<int, int>> moves = getLegalMoves(row, col);
                    for (const auto& move : moves) {
                        possibleMoves.insert(move);
                    }
                }
            }
        }
    }

    // 🔹 Debug kiírás, hogy tényleg frissült-e
    qDebug() << "♟️ Updated possibleMoves:";
    for (const auto& move : possibleMoves) {
        qDebug() << "(" << move.first << "," << move.second << ")";
    }
}

bool Widget::applyMove(QString move)
{
    qDebug() << "?? Applying move: " << move;

    auto coords = convertUciToCoords(move);
    int fromRow = coords.first.first;
    int fromCol = coords.first.second;
    int toRow = coords.second.first;
    int toCol = coords.second.second;

    qDebug() << "?? From (" << fromRow << "," << fromCol << ") to (" << toRow << "," << toCol << ")";

    // Ellenőrizzük, hogy a tábla érvényes helyére lépünk-e
    if (fromRow < 0 || fromRow >= 8 || fromCol < 0 || fromCol >= 8 ||
        toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
        qFatal("❌ Invalid move: coordinates out of bounds!");
        qDebug() << "❌ Invalid move: coordinates out of bounds!";
        return false;
    }

    char piece = board[fromRow][fromCol];

    // A felhasználó csak fehérrel léphet
    if (isWhiteTurn && (piece >= 'a' && piece <= 'z')) {
        qFatal("❌ You can only move white pieces!");
        qDebug() << "❌ You can only move white pieces!";
        return false;
    }

    // A sakkmotor csak feketével léphet
    if (!isWhiteTurn && (piece >= 'A' && piece <= 'Z')) {
        qFatal("❌ Engine can only move black pieces!");
        qDebug() << "❌ Engine can only move black pieces!";
        return false;
    }

    // Ellenőrizzük, hogy a lépés legális-e
    QVector<QPair<int, int>> legalMoves = getLegalMoves(fromRow, fromCol);

    // ⛔ **HIBA MEGOLDÁSA:** Ha üres a lista, ne próbáljuk elérni az elemeit!
    if (legalMoves.isEmpty()) {
        qFatal("❌ No legal moves found for this piece!");
        qDebug() << "❌ No legal moves found for this piece!";
        return false;
    }

    if (!legalMoves.contains(QPair<int, int>(toRow, toCol))) {
        qFatal("❌ Invalid move! Move is not in legalMoves.");
        qDebug() << "❌ Invalid move! Move is not in legalMoves.";
        return false;
    }

    // Ha a lépés érvényes, végrehajtjuk
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = ' ';
    moveHistory.append(move);
    uciEngine->setPosition(moveHistory);
    qDebug() << "📜 Move history sent to engine: " << moveHistory;
    isWhiteTurn = !isWhiteTurn;
    possibleMoves.clear();
    update();

    // Ha a felhasználó lépett, akkor a motor jön
    if (!isWhiteTurn) {
        stepsCount++;
        ui->stepLabel->setText(QString("Steps Count: %1").arg(stepsCount));
        uciEngine->requestBestMove(1000);
    }

    return true;
}

void Widget::clearPieceCount()
{
    int whiteCount = 16, blackCount = 16;
    int whiteCaptured = 0, blackCaptured = 0;
    ui->whitePiecesLabel->setText(QString("White Pieces: %1").arg(whiteCount));
    ui->blackPiecesLabel->setText(QString("Black Pieces: %1").arg(blackCount));
    ui->whiteCapturedLabel->setText(QString("White Pieces Knocked Out: %1").arg(whiteCaptured));
    ui->blackCapturedLabel->setText(QString("Black Pieces Knocked Out: %1").arg(blackCaptured));
}

void Widget::clearStepsCounter()
{
    stepsCount = 0;
    ui->stepLabel->setText(QString("Steps Count: %1").arg(stepsCount));
}

void Widget::startNewGame()
{
    isStarted = true;
    update();
    uciEngine->startNewGame();
}

void Widget::resetGame()
{
    moveHistory.clear();  // Történet törlése
    clearPieceCount();
    clearStepsCounter();
    initializeBoard();  // Tábla alaphelyzetbe állítása
    possibleMoves.clear();
    selectedRow = -1;
    selectedCol = -1;
    update();
}

bool Widget::isEnemyPiece(int row, int col)
{
    char piece = board[row][col];
    if (piece == ' ' || (isWhiteTurn && piece >= 'A' && piece <= 'Z') ||
        (!isWhiteTurn && piece >= 'a' && piece <= 'z')) {
        return false; // Ha a mező üres vagy a saját színű bábu, nem támadható
    }
    return true; // Ellenfél bábujának támadása
}
