#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mpvhandler.h"
#include "util.h"

#include <QMessageBox>

BakaEngine::BakaEngine(QObject *parent):
    QObject(parent)
{
    window = static_cast<MainWindow*>(parent);
    mpv = new MpvHandler(window->ui->mpvFrame->winId(), this);
    settings = Util::InitializeSettings(this);
}

BakaEngine::~BakaEngine()
{
    delete mpv;
    delete settings;
}

void BakaEngine::LoadSettings()
{
    if(settings == nullptr)
        return;

    settings->Load();

    QString version;
    if(settings->isEmpty()) // empty settings
    {
        version = "2.0.2"; // current version

        // populate initially
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
        settings->beginGroup("mpv");
        settings->setValueQStringList("vo", QStringList{"vdpau","opengl-hq"});
        settings->setValue("hwdec", "auto");
        settings->endGroup();
#endif
    }
    else
    {
        settings->beginGroup("baka-mplayer");
        version = settings->value("version", "1.9.9"); // defaults to the first version without version info in settings
        settings->endGroup();
    }

    if(version == "2.0.2") Load2_0_2();
    else if(version == "2.0.1") { Load2_0_1(); settings->clear(); SaveSettings(); }
    else if(version == "2.0.0") { Load2_0_0(); settings->clear(); SaveSettings(); }
    else if(version == "1.9.9") { Load1_9_9(); settings->clear(); SaveSettings(); }
    else
    {
        Load2_0_2();
        window->ui->action_Preferences->setEnabled(false);
        QMessageBox::information(window, tr("Settings version not recognized"), tr("The settings file was made by a newer version of baka-mplayer; please upgrade this version or seek assistance from the developers.\nSome features may not work and changed settings will not be saved."));
        delete settings;
        settings = nullptr;
    }
}

void BakaEngine::SaveSettings()
{
    if(settings == nullptr)
        return;

    settings->beginGroup("baka-mplayer");
    settings->setValue("onTop", window->onTop);
    settings->setValueInt("autoFit", window->autoFit);
    settings->setValueBool("trayIcon", window->sysTrayIcon->isVisible());
    settings->setValueBool("hidePopup", window->hidePopup);
    settings->setValueBool("remaining", window->remaining);
    settings->setValueInt("splitter", (window->ui->splitter->position() == 0 ||
                                    window->ui->splitter->position() == window->ui->splitter->max()) ?
                                    window->ui->splitter->normalPosition() :
                                    window->ui->splitter->position());
    settings->setValueBool("showAll", !window->ui->hideFilesButton->isChecked());
    settings->setValueBool("screenshotDialog", window->screenshotDialog);
    settings->setValueBool("debug", window->debug);
    settings->setValueQStringList("recent", window->recent);
    settings->setValueInt("maxRecent", window->maxRecent);
    settings->setValue("lang", window->lang);
    settings->setValueBool("gestures", window->gestures);
    settings->setValue("version", "2.0.2");
    settings->endGroup();

    settings->beginGroup("mpv");
    settings->setValueInt("volume", mpv->volume);
    settings->setValueDouble("speed", mpv->speed);
    if(mpv->screenshotFormat != "")
        settings->setValue("screenshot-format", mpv->screenshotFormat);
    if(mpv->screenshotTemplate != "")
        settings->setValue("screenshot-template", mpv->screenshotDir+"/"+mpv->screenshotTemplate);
    settings->endGroup();

    settings->Save();
}