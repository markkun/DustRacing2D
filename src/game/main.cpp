// This file is part of Dust Racing 2D.
// Copyright (C) 2011 Jussi Lind <jussi.lind@iki.fi>
//
// Dust Racing 2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// Dust Racing 2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dust Racing 2D. If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QHBoxLayout>
#include <QMessageBox>

#include "game.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "settings.hpp"

#include <MCException>
#include <MCLogger>

static const char * INIT_ERROR = "Initing the game failed!";

static void initLogger()
{
    QString logPath = QDir::tempPath() + QDir::separator() + "dustrac.log";
    MCLogger::init(logPath.toStdString().c_str());
    MCLogger::setEchoMode(true);
    MCLogger::setDateTime(true);
}

static void checkOpenGLVersion()
{
    if (QGLFormat::openGLVersionFlags() < QGLFormat::OpenGL_Version_3_0)
    {
        QString versionError = QObject::tr("At least OpenGL 3.0 is required!");
        QMessageBox::critical(nullptr, QObject::tr("Cannot start Dust Racing 2D"), versionError);
        throw MCException(versionError.toStdString());
    }
}

int main(int argc, char ** argv)
{
    try
    {
        Settings settings;

        // Create the QApplication
#ifndef USE_QT5
        QApplication::setGraphicsSystem("opengl");
#endif
        QApplication app(argc, argv);

        initLogger();

        checkOpenGLVersion();

        // Create the main window / renderer
        int hRes, vRes;
        bool fullResolution;
        bool windowed = false;
        Settings::instance().loadResolution(hRes, vRes, fullResolution);

        if (fullResolution)
        {
            hRes = QApplication::desktop()->width();
            vRes = QApplication::desktop()->height();
        }

        MCLogger().info() << "Resolution: " << hRes << " " << vRes << " " << fullResolution;

        QGLFormat qglFormat;
        qglFormat.setVersion(3, 0);
        qglFormat.setProfile(QGLFormat::CoreProfile);
        qglFormat.setSampleBuffers(false);

        MCLogger().info() << "Creating the renderer..";
        Renderer renderer(qglFormat, hRes, vRes, fullResolution, windowed);
        renderer.activateWindow();

        // Adjust scene height so that window aspect ratio is taken into account.
        if (!windowed)
        {
            const int newSceneHeight =
                Scene::width() * QApplication::desktop()->height() / QApplication::desktop()->width();
            Scene::setSize(Scene::width(), newSceneHeight);

            renderer.showFullScreen();
        }
        else
        {
            const int newSceneHeight = Scene::width() * vRes / hRes;
            Scene::setSize(Scene::width(), newSceneHeight);

            renderer.show();
        }

        renderer.setFocus();
        renderer.setCursor(Qt::BlankCursor);

        // Create the game object and set the renderer
        MCLogger().info() << "Creating game object..";
        Game game;
        game.setRenderer(&renderer);
        game.setFps(Settings::instance().loadFps());

        // Initialize and start the game
        if (game.init())
        {
            game.start();
        }
        else
        {
            MCLogger().fatal() << INIT_ERROR;
            return EXIT_FAILURE;
        }

        return app.exec();
    }
    // Catch some errors during game initialization e.g.
    // a vertex shader not found.
    catch (MCException & e)
    {
        MCLogger().fatal() << e.what();
        MCLogger().fatal() << INIT_ERROR;
        return EXIT_FAILURE;
    }
}
