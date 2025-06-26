#include "UCIEngine.h"
#include <QDebug>
#include <QThread>
#include <QDir>

UCIEngine::UCIEngine(QObject *parent) : QObject(parent), uciProcess(new QProcess(this)) // Dinamikusan létrehozott QProcess
{
    enginePath = QDir::toNativeSeparators("C:/Users/Mark/Documents/grafikus_felhasznaloi_feluletek_fejlesztese_C++_nyelven/chess/stockfish/stockfish/stockfish-windows-x86-64-avx2.exe");
    uciProcess->setProgram(enginePath);
    uciProcess->setProcessChannelMode(QProcess::SeparateChannels);
}

UCIEngine::~UCIEngine()
{
    sendCommand("quit");
    uciProcess->waitForFinished();
    delete uciProcess; // Memória felszabadítása
}

void UCIEngine::startEngine()
{
    if (uciProcess->state() == QProcess::Running) {
        qDebug() << "⚠️ A sakkmotor már fut!";
        return;
    }
    uciProcess->start();
    if (!uciProcess->waitForStarted(5000)) {
        qDebug() << "❌ Nem sikerült elindítani a sakkmotort! Hiba: " << uciProcess->errorString();
        return;
    }
    QThread::msleep(500);
    sendCommand("uci");
}

void UCIEngine::sendCommand(const QString &command)
{
    if (uciProcess->state() == QProcess::Running) {
        uciProcess->write(command.toUtf8() + "\n");
        uciProcess->waitForBytesWritten();
    } else {
        qDebug() << "⚠️ A sakkmotor nem fut!";
    }
}

void UCIEngine::startNewGame()
{
    sendCommand("ucinewgame");
    sendCommand("isready");
}

void UCIEngine::setPosition(const QStringList &moves)
{
    QString positionCommand = "position startpos";
    if (!moves.isEmpty()) {
        positionCommand += " moves " + moves.join(" ");
    }
    sendCommand(positionCommand);
}

void UCIEngine::requestBestMove(int movetime)
{
    sendCommand(QString("go movetime %1").arg(movetime));
}

void UCIEngine::handleEngineOutput()
{
    while (uciProcess->canReadLine()) {
        QString response = uciProcess->readLine().trimmed();
        qDebug() << "UCI Engine válasz: " << response;

        if (response == "uciok") {
            qDebug() << "✅ UCI motor készen áll.";
        }
        else if (response == "readyok") {
            qDebug() << "✅ Motor készen áll a parancsokra.";
        }
        else if (response.startsWith("bestmove")) {
            QString bestMove = response.split(" ").value(1, "");
            emit bestMoveFound(bestMove);
        }
    }
}

void UCIEngine::handleEngineErrorOutput()
{
    while (uciProcess->canReadLine()) {
        QString errorOutput = uciProcess->readLine().trimmed();
        qDebug() << "❌ Engine hiba: " << errorOutput;
    }
}

void UCIEngine::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "⚠️ A sakkmotor leállt. Kilépési kód: " << exitCode;

    if (exitStatus == QProcess::CrashExit) {
        qDebug() << "❌ A sakkmotor összeomlott!";
    } else {
        qDebug() << "✅ A sakkmotor normálisan leállt.";
    }
}
