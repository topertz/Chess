#ifndef UCIENGINE_H
#define UCIENGINE_H

#include <QObject>
#include <QProcess>

class UCIEngine : public QObject
{
    Q_OBJECT
public:
    explicit UCIEngine(QObject *parent = nullptr);
    ~UCIEngine();
    void handleEngineOutput();
    void handleEngineErrorOutput();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void startEngine();
    void sendCommand(const QString &command);
    void startNewGame();
    void setPosition(const QStringList &moves);
    void requestBestMove(int movetime = 1000);
    QProcess *uciProcess;

signals:
    void bestMoveFound(QString bestMove);

private:
    QString enginePath;
};

#endif // UCIENGINE_H
