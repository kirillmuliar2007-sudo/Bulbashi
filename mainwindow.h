#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSoundEffect>
#include <QList>
#include <QTimer>
#include <QTextToSpeech>
#include "person.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNextWordButtonClicked();
    void updateRhymeWord();

private:
    QTextToSpeech *textToSpeech;
    Ui::MainWindow *ui;
    QLabel *longestWordLabel;
    QLabel *currentRhymeWordLabel;
    QPushButton *nextWordButton;
    QList<Person> persons;
    QStringList rhymeWords;
    int currentIndex;
    int currentWordIndex;
    QTimer *rhymeTimer;
    bool rhymeRunning;
    QLabel *highlightedLabel;
    void rearrangeCircle();

    QVector<QString> loadRhymes(const QString &filePath);
    QList<Person> loadPhotos(const QString &folderPath);
    void displayPhotosInCircle(const QList<Person> &persons);
    QString selectRandomRhyme(const QVector<QString> &rhymes);
    QString findLongestWord(const QString &rhyme);
    void highlightCurrentPerson();
    void removeCurrentPerson();
    void playSoundEffect(const QString &soundPath);
    void animateRemoval(QLabel *label);
    void displayWinner(QLabel *winnerLabel);
};

#endif // MAINWINDOW_H
