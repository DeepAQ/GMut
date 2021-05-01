#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>

#ifdef Q_OS_WIN
const auto executableName = "mut.exe";
#else
const auto executableName = "mut";
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    this->process = new QProcess(this);
    this->process->setProgram(QApplication::applicationDirPath() + '/' + executableName);
    this->process->setWorkingDirectory(QApplication::applicationDirPath());
    this->process->setArguments({"-stdin"});
    QObject::connect(this->process, &QProcess::stateChanged, this, &MainWindow::processStateChanged);
    QObject::connect(this->process, &QProcess::readyReadStandardOutput, this, &MainWindow::processReadyReadOutput);
    QObject::connect(this->process, &QProcess::readyReadStandardError, this, &MainWindow::processReadyReadError);
    this->refreshAutoStart();
    this->loadConfig();
}

MainWindow::~MainWindow()
{
    if (this->process != nullptr) {
        delete this->process;
    }
    if (this->ui != nullptr) {
        delete this->ui;
    }
}

void MainWindow::loadConfig()
{
    QFile configFile(QApplication::applicationDirPath() + "/config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        ui->textLogs->setPlainText("Unable to load configuration file.");
        return;
    }

    auto doc = QJsonDocument::fromJson(configFile.readAll());
    configFile.close();
    if (doc.isArray()) {
        ui->comboCfg->clear();
        auto arr = doc.array();
        for (auto value : arr) {
            if (value.isObject()) {
                auto obj = value.toObject();
                if (obj.contains("name") && obj.contains("args")) {
                    auto name = obj["name"].toString();
                    auto args = obj["args"].toString();
                    ui->comboCfg->addItem(name, args);
                }
            }
        }
    }
}

void MainWindow::refreshAutoStart()
{
#ifdef Q_OS_WIN
    QSettings key("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    this->ui->chkAutoStart->setChecked(key.contains("GMut"));
    this->ui->chkAutoStart->setEnabled(true);
#endif
}

void MainWindow::addAutoStart()
{
#ifdef Q_OS_WIN
    QSettings key("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    key.setValue("GMut", QDir::toNativeSeparators(QApplication::applicationFilePath()));
#endif
}

void MainWindow::removeAutoStart()
{
#ifdef Q_OS_WIN
    QSettings key("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    key.remove("GMut");
#endif
}

void MainWindow::processStateChanged(QProcess::ProcessState state)
{
    this->ui->btnStart->setEnabled(state == QProcess::ProcessState::NotRunning);
    this->ui->btnStop->setEnabled(state == QProcess::ProcessState::Running);
}

void MainWindow::processReadyReadOutput()
{
    this->ui->textLogs->appendPlainText(this->process->readAllStandardOutput().trimmed());
}

void MainWindow::processReadyReadError()
{
    this->ui->textLogs->appendPlainText(this->process->readAllStandardError().trimmed());
}

void MainWindow::on_btnReload_clicked()
{
    this->loadConfig();
}

void MainWindow::on_btnStart_clicked()
{
    if (this->process->state() != QProcess::ProcessState::NotRunning) {
        return;
    }

    if (!QFile::exists(this->process->program())) {
        QMessageBox::critical(this, "Error", "Mut executable file not found!");
        return;
    }

    auto i = this->ui->comboCfg->currentIndex();
    if (i >= this->ui->comboCfg->count()) {
        QMessageBox::critical(this, "Error", "No configuration is selected!");
        return;
    }

    this->ui->textLogs->clear();
    this->process->start();
    this->process->write(this->ui->comboCfg->itemData(i).toByteArray());
    this->process->closeWriteChannel();
}

void MainWindow::on_btnStop_clicked()
{
    if (this->process->state() == QProcess::ProcessState::NotRunning) {
        return;
    }

    this->process->kill();
}

void MainWindow::on_comboCfg_currentIndexChanged(int)
{
    this->on_btnStop_clicked();
    this->process->waitForFinished();
    this->on_btnStart_clicked();
}

void MainWindow::on_chkAutoStart_clicked()
{
    if (this->ui->chkAutoStart->isChecked()) {
        this->addAutoStart();
    } else {
        this->removeAutoStart();
    }
    this->refreshAutoStart();
}
