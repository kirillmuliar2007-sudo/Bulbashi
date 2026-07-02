#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <cmath>
#include <QMessageBox>
#include <QTextToSpeech>
#include <QTimer>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentIndex(0)
    , currentWordIndex(0)
    , rhymeRunning(false)
    , highlightedLabel(nullptr)
{
    ui->setupUi(this);
    setFixedSize(800, 600);
    QRandomGenerator::securelySeeded();
    QVector<QString> rhymes = loadRhymes(QDir::currentPath() + "/texts/input.txt");
    persons = loadPhotos(QDir::currentPath() + "/photos");

    if (persons.isEmpty() || rhymes.isEmpty()) {
        QMessageBox::critical(this, "Critical Error", "No photos or rhymes found. The application will now exit.");
        QTimer::singleShot(0, this, &QCoreApplication::quit);
        return;
    }
    longestWordLabel = new QLabel(this);
    longestWordLabel->setGeometry(110, 10, 400, 25);
    longestWordLabel->setFont(QFont("Arial", 11, QFont::Bold));
    longestWordLabel->setStyleSheet("color: #2E8B57;");
    longestWordLabel->setText("Нажмите Старт для начала игры");
    longestWordLabel->show();
    currentRhymeWordLabel = new QLabel(this);
    currentRhymeWordLabel->setAlignment(Qt::AlignCenter);
    currentRhymeWordLabel->setFont(QFont("Arial", 15));
    currentRhymeWordLabel->setFixedWidth(400);
    currentRhymeWordLabel->move(width() / 2 - 200, height() / 2 + 30);
    currentRhymeWordLabel->hide();
    displayPhotosInCircle(persons);
    nextWordButton = new QPushButton("Старт", this);
    nextWordButton->setGeometry(width() / 2 - 100, height() - 50, 200, 40);
    nextWordButton->setFont(QFont("Arial", 11, QFont::Bold));
    connect(nextWordButton, &QPushButton::clicked, this, &MainWindow::onNextWordButtonClicked);
    nextWordButton->show();
    nextWordButton->raise();
    textToSpeech = new QTextToSpeech(this);
    textToSpeech->setVolume(1.0);
    rhymeTimer = new QTimer(this);
    connect(rhymeTimer, &QTimer::timeout, this, &MainWindow::updateRhymeWord);
        QPushButton *helpButton = new QPushButton("Справка", this);
    helpButton->setGeometry(10, 10, 80, 25);
    helpButton->setFont(QFont("Arial", 9));
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::onHelpButtonClicked);
    helpButton->show();
    helpButton->raise();

}

MainWindow::~MainWindow() {
    delete ui;
}

QVector<QString> MainWindow::loadRhymes(const QString &filePath) {
    QVector<QString> rhymes;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            line.remove(QRegularExpression("[[:punct:]]"));
            rhymes.append(line);
        }
        file.close();
    }
    return rhymes;
}

QList<Person> MainWindow::loadPhotos(const QString &folderPath) {
    QList<Person> loadedPersons;
    QDir dir(folderPath);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        QPixmap pixmap(fileInfo.absoluteFilePath());
        if (!pixmap.isNull()) {
            loadedPersons.append(Person(fileInfo.baseName(), pixmap));
        }
    }
    return loadedPersons;
}

void MainWindow::displayPhotosInCircle(const QList<Person> &circlePersons) {
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(centerX, centerY) - 110;
    int count = circlePersons.size();
    if (count == 1) {
        int x = centerX - 50;
        int y = centerY - 50;

        QLabel *label = new QLabel(this);
        label->setPixmap(circlePersons[0].photo.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        label->setFixedSize(100, 100);
        label->move(x, y);
        label->setAlignment(Qt::AlignCenter);
        label->setObjectName(circlePersons[0].name);
        label->show();
        QLabel *nameLabel = new QLabel(circlePersons[0].name, this);
        nameLabel->setFont(QFont("Arial", 9, QFont::Bold));
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setGeometry(x - 30, y + 105, 160, 20);
        nameLabel->setStyleSheet("color: black; background: transparent;");
        nameLabel->setObjectName(circlePersons[0].name + "_name");
        nameLabel->show();
        nameLabel->raise();
        return;
    }

    for (int i = 0; i < count; ++i) {
        double angle = (2 * M_PI / count) * i - M_PI / 2;
        int x = centerX + radius * cos(angle) - 50;
        int y = centerY + radius * sin(angle) - 50;
        QLabel *label = new QLabel(this);
        label->setPixmap(circlePersons[i].photo.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        label->setFixedSize(100, 100);
        label->move(x, y);
        label->setAlignment(Qt::AlignCenter);
        label->setObjectName(circlePersons[i].name);
        label->show();
    }

    for (int i = 0; i < count; ++i) {
        double angle = (2 * M_PI / count) * i - M_PI / 2;
        int x = centerX + radius * cos(angle) - 50;
        int y = centerY + radius * sin(angle) - 50;
        QLabel *nameLabel = new QLabel(circlePersons[i].name, this);
        nameLabel->setFont(QFont("Arial", 9, QFont::Bold));
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setGeometry(x - 30, y + 105, 160, 20);
        nameLabel->setStyleSheet("color: black; background: transparent;");
        nameLabel->setObjectName(circlePersons[i].name + "_name");
        nameLabel->show();
        nameLabel->raise();

        QLabel *wordLabel = new QLabel(this);
        wordLabel->setFont(QFont("Arial", 10, QFont::Bold));
        wordLabel->setAlignment(Qt::AlignCenter);
        wordLabel->setGeometry(x - 30, y + 125, 160, 20);
        wordLabel->setStyleSheet("color: #1E90FF; background: transparent;");
        wordLabel->setObjectName(circlePersons[i].name + "_word");
        wordLabel->show();
        wordLabel->raise();
    }
}

void MainWindow::rearrangeCircle() {
    QList<QLabel*> allLabels = this->findChildren<QLabel*>();
    for (QLabel* label : allLabels) {
        if (!label->objectName().isEmpty()) {
            if (label != longestWordLabel && label != currentRhymeWordLabel) {
                label->deleteLater();
            }
        }
    }
    displayPhotosInCircle(persons);
    if (nextWordButton) {
        nextWordButton->raise();
    }
}
QString MainWindow::selectRandomRhyme(const QVector<QString> &rhymes) {
    if (rhymes.isEmpty()) {
        return QString();
    }
    int index = QRandomGenerator::global()->bounded(rhymes.size());
    return rhymes[index];
}
QString MainWindow::findLongestWord(const QString &rhyme) {
    QStringList words = rhyme.split(QRegularExpression("\\s+"));
    QString longestWord;
    for (const QString &word : words) {
        if (word.length() > longestWord.length()) {
            longestWord = word;
        }
    }
    return longestWord;
}

void MainWindow::onNextWordButtonClicked() {
    if (persons.isEmpty()) {
        return;
    }
    if (!rhymeRunning) {
        static QVector<QString> allRhymes = loadRhymes(QDir::currentPath() + "/texts/input.txt");
        if (allRhymes.isEmpty()) return;

        QString currentRhyme = selectRandomRhyme(allRhymes);
        rhymeWords = currentRhyme.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (rhymeWords.isEmpty()) return;

        longestWordLabel->setText("Самое длинное слово: " + findLongestWord(currentRhyme));
        longestWordLabel->show();

        rhymeRunning = true;
        currentWordIndex = 0;
        currentRhymeWordLabel->setText(rhymeWords[0]);
        currentRhymeWordLabel->show();
        highlightCurrentPerson();
        QString currentWord = rhymeWords[currentWordIndex];
        textToSpeech->say(currentWord);
        rhymeTimer->start(1500);
    }
}

void MainWindow::updateRhymeWord() {
    if (persons.isEmpty()) {
        rhymeTimer->stop();
        return;
    }

    if (highlightedLabel) {
        highlightedLabel->setStyleSheet("");
        QLabel *prevWordLabel = findChild<QLabel *>(persons[currentIndex].name + "_word");
        if (prevWordLabel) {
            prevWordLabel->clear();
        }
    }

    currentWordIndex = (currentWordIndex + 1) % rhymeWords.size();
    currentRhymeWordLabel->setText(rhymeWords[currentWordIndex]);
    currentIndex = (currentIndex + 1) % persons.size();
    highlightCurrentPerson();
    QString currentWord = rhymeWords[currentWordIndex];
    textToSpeech->say(currentWord);

    if (currentWordIndex == rhymeWords.size() - 1) {
        rhymeTimer->stop();
        QTimer::singleShot(1000, this, [this]() {
            removeCurrentPerson();
            currentRhymeWordLabel->hide();
            rhymeRunning = false;
        });
    }
}

void MainWindow::highlightCurrentPerson() {
    if (persons.isEmpty() || currentIndex >= persons.size()) return;

    highlightedLabel = findChild<QLabel *>(persons[currentIndex].name);
    if (highlightedLabel) {
        highlightedLabel->setStyleSheet("border: 3px solid red; border-radius: 5px;");
    }

    QLabel *wordLabel = findChild<QLabel *>(persons[currentIndex].name + "_word");
    if (wordLabel && currentWordIndex < rhymeWords.size()) {
        wordLabel->setText(rhymeWords[currentWordIndex]);
    }
}

void MainWindow::removeCurrentPerson() {
    if (persons.isEmpty() || currentIndex >= persons.size()) return;

QString personName = persons[currentIndex].name;QLabel *labelToRemove = findChild<QLabel *>(personName);
QLabel *nameToRemove = findChild<QLabel *>(personName + "_name");
QLabel *wordToRemove = findChild<QLabel *>(personName + "_word");
if (nameToRemove) nameToRemove->deleteLater();if (wordToRemove) wordToRemove->deleteLater();
if (labelToRemove) {animateRemoval(labelToRemove);}persons.removeAt(currentIndex);
if (persons.isEmpty()) {currentIndex = 0;
}
else {currentIndex = currentIndex % persons.size();
}
if (persons.size() > 1) {
    QTimer::singleShot(1050, this, [this]() {
        rearrangeCircle();
    });
}
if (persons.size() > 1) {
    QTimer::singleShot(1050, this, [this]() {
        rearrangeCircle();
    });
}
else if (persons.size() == 1) {
    QTimer::singleShot(1050, this, [this]() {
        rearrangeCircle();
        QLabel *winnerLabel = findChild<QLabel *>(persons[0].name);
        if (winnerLabel) {
            displayWinner(winnerLabel);
        }
    });
}
}

void MainWindow::playSoundEffect(const QString &soundPath) {
    static QMediaPlayer *outPlayer = nullptr;
    static QAudioOutput *outAudio = nullptr;
    static QMediaPlayer *winPlayer = nullptr;
    static QAudioOutput *winAudio = nullptr;

    if (soundPath.contains("out.wav")) {
        if (!outPlayer) {
            outPlayer = new QMediaPlayer(this);
            outAudio = new QAudioOutput(this);
            outPlayer->setAudioOutput(outAudio);
            outAudio->setVolume(1.0);
            outPlayer->setSource(QUrl::fromLocalFile(QDir::currentPath() + "/sounds/out.wav"));
        }
        outPlayer->stop();
        outPlayer->play();
    }
    else if (soundPath.contains("fireworks.wav")) {
        if (!winPlayer) {
            winPlayer = new QMediaPlayer(this);
            winAudio = new QAudioOutput(this);
            winPlayer->setAudioOutput(winAudio);
            winAudio->setVolume(1.0);
            winPlayer->setSource(QUrl::fromLocalFile(QDir::currentPath() + "/sounds/fireworks.wav"));
        }
        winPlayer->stop();
        winPlayer->play();
    }
}

void MainWindow::animateRemoval(QLabel *label) {
    if (!label) return;
    QPropertyAnimation *animation = new QPropertyAnimation(label, "pos", this);
    animation->setDuration(1000);
    animation->setStartValue(label->pos());
    animation->setEndValue(QPoint(label->pos().x() + 350, -250));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    playSoundEffect("sounds/out.wav");
}
void MainWindow::displayWinner(QLabel *winnerLabel) {
    Q_UNUSED(winnerLabel);
    int centerX = width() / 2;
    int centerY = height() / 2;
    int photoY = centerY - 50;
    if (persons.isEmpty()) return;
    if (nextWordButton) nextWordButton->hide();
    if (currentRhymeWordLabel) currentRhymeWordLabel->hide();
    QPixmap crown(QDir::currentPath() + "/images/crown.png");
    if (!crown.isNull()) {
        QLabel *crownLabel = new QLabel(this);
        crownLabel->setPixmap(crown.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        crownLabel->move(centerX - 25, photoY - 60);
        crownLabel->show();
    }
    QLabel *winnerTextLabel = new QLabel(this);
    winnerTextLabel->setText("ПОБЕДИТЕЛЬ");
    winnerTextLabel->setFont(QFont("Arial", 30, QFont::Bold));
    winnerTextLabel->setAlignment(Qt::AlignCenter);
    winnerTextLabel->setStyleSheet("color: #FFD700; background: transparent;");
    winnerTextLabel->setGeometry(centerX - 200, photoY + 130, 400, 60);
    winnerTextLabel->show();
    playSoundEffect("sounds/fireworks.wav");
}
void MainWindow::onHelpButtonClicked() {
    QMessageBox::about(this, "Справка о создателе",
                      "<h3>Лабораторная работа \"Считалочка\"</h3>"
                       "<p><b>Создатель:</b> Муляр Кирилл и Александр Гордейко</p>"
                     "<p><b>Группа:</b> 7</p>"
                       "<p>Программа разработана на C++ с использованием фреймворка Qt.</p>");
}
