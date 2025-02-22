/*
The MIT License (MIT)

Copyright © 2022-2023 Antonio Dias (https://github.com/antonypro)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifdef _WIN32

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <windows.h>
#include <dwmapi.h>

#endif

#include "mainwindow.h"

#ifdef QGOODWINDOW
inline qreal pixelRatio()
{
    QScreen *screen = QApplication::primaryScreen();

#ifdef Q_OS_MAC
    qreal pixel_ratio = screen->devicePixelRatio();
#else
    qreal pixel_ratio = screen->logicalDotsPerInch() / qreal(96);
#endif

    return pixel_ratio;
}
#endif

MainWindow::MainWindow(QWidget *parent) : QGoodWindow(parent)
{
    m_central_widget = new CentralWidget(this);
    m_central_widget->setWindowFlags(Qt::Widget);

    m_good_central_widget = new QGoodCentralWidget(this);

#ifdef QGOODWINDOW

    //macOS uses global menu bar
#ifndef Q_OS_MAC
    QMenuBar *menu_bar = m_central_widget->menuBar();

    //Set font of menu bar
    QFont font = menu_bar->font();
    font.setPixelSize(qRound(12 * pixelRatio()));
#ifdef Q_OS_WIN
    font.setFamily("Segoe UI");
#else
    font.setFamily(qApp->font().family());
#endif
    menu_bar->setFont(font);

    QTimer::singleShot(0, this, [=]{
        menu_bar->setFixedWidth(menu_bar->sizeHint().width());
        const int title_bar_height = m_good_central_widget->titleBarHeight();
        menu_bar->setStyleSheet(QString("QMenuBar {height: %0px;}").arg(title_bar_height));
    });

    m_good_central_widget->setLeftTitleBarWidget(menu_bar);
#endif

    QWidget *widget = new QWidget();

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    QPushButton *button = new QPushButton("Toggle All!", widget);

    connect(button, &QPushButton::clicked, this, [=]{
        m_good_central_widget->setTitleVisible(!m_good_central_widget->isTitleVisible());
        m_good_central_widget->setIconVisible(!m_good_central_widget->isIconVisible());
        m_good_central_widget->setTitleBarColor(!m_good_central_widget->isTitleVisible() ? Qt::red : QColor());
        m_good_central_widget->setActiveBorderColor(!m_good_central_widget->isTitleVisible() ? Qt::red : QColor());
    });

    QLabel *label = new QLabel("Label", widget);
    label->setAttribute(Qt::WA_TransparentForMouseEvents);

    layout->addWidget(button);
    layout->addWidget(label);

    m_good_central_widget->setRightTitleBarWidget(widget, true);
#endif

    connect(qGoodStateHolder, &QGoodStateHolder::currentThemeChanged, this, &MainWindow::themeChange);

    connect(this, &QGoodWindow::systemThemeChanged, this, [=]{
        qGoodStateHolder->setCurrentThemeDark(QGoodWindow::isSystemThemeDark());
    });

    QShortcut *shortcut1 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);

    connect(shortcut1, &QShortcut::activated, this, [=]{
        qGoodStateHolder->setCurrentThemeDark(!qGoodStateHolder->isCurrentThemeDark());
    });

    QShortcut *shortcut2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this);

    connect(shortcut2, &QShortcut::activated, this, [=]{
        m_good_central_widget->setTitleBarVisible(!m_good_central_widget->isTitleBarVisible());
    });

    qGoodStateHolder->setCurrentThemeDark(QGoodWindow::isSystemThemeDark());

    m_good_central_widget->setCentralWidget(m_central_widget);
    setCentralWidget(m_good_central_widget);

    setWindowIcon(qApp->style()->standardIcon(QStyle::SP_DesktopIcon));
    setWindowTitle("Good Window - CTRL+S toggle theme - CTRL+T toggle title bar!");

    resize(m_central_widget->size());
    move(QGuiApplication::primaryScreen()->availableGeometry().center() - rect().center());

#ifdef QGOODWINDOW
    m_good_central_widget->setTitleAlignment(Qt::AlignCenter);
#endif
}

MainWindow::~MainWindow()
{

}

void MainWindow::themeChange()
{
    if (qGoodStateHolder->isCurrentThemeDark())
        setAppDarkTheme();
    else
        setAppLightTheme();
}

#ifdef QT_VERSION_QT5
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
#ifdef QT_VERSION_QT6
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#endif
{
#ifdef QGOODWINDOW
#ifdef Q_OS_WIN
#ifdef QT_VERSION_QT5
    MSG *msg = static_cast<MSG*>(message);

    switch (msg->message)
    {
    case WM_THEMECHANGED:
    case WM_DWMCOMPOSITIONCHANGED:
    {
        //Keep window theme on Windows theme change events.
        QTimer::singleShot(100, this, [=]{
            if (qGoodStateHolder->isCurrentThemeDark())
                setAppDarkTheme();
            else
                setAppLightTheme();
        });

        break;
    }
    default:
        break;
    }
#endif
#endif
#endif
    return QGoodWindow::nativeEvent(eventType, message, result);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgbox(this);
    msgbox.setIcon(QMessageBox::Question);
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgbox.setDefaultButton(QMessageBox::No);
    msgbox.setText("Are you sure to close?");

    int result = QGoodCentralWidget::execDialogWithWindow(&msgbox, this, m_good_central_widget);

    if (result != QMessageBox::Yes)
    {
        event->ignore();
        return;
    }
}
