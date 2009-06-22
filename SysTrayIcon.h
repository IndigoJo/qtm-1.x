/*********************************************************************************

    SysTrayIcon.h - Header file for QTM system tray icon
    Copyright (C) 2006-2009 Matthew J Smith

    This file is part of QTM.

    QTM is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License (version 2), as 
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*******************************************************************************/

#ifndef SYSTRAYICON_H
#define SYSTRAYICON_H

#include <QtGlobal>

#if QT_VERSION >= 0x040200

// #include "useSTI.h"
//#ifdef USE_SYSTRAYICON

#include <QSystemTrayIcon>
#include <QByteArray>
#include <QString>
#include <QClipboard>
#include <QList>
#include <QStringList>
#include <QDomElement>
#include "Application.h"
class QMenu;
class QAction;
class QHttpResponseHeader;
class QuickpostTemplate;

#ifdef USE_SAFEHTTP
class SafeHttp;
#define _HTTP SafeHttp
#else
class QHttp;
#define _HTTP QHttp
#endif

#ifdef Q_WS_MAC
extern void qt_mac_set_dock_menu( QMenu * );
#define STI_SUPERCLASS QObject
#else
#define STI_SUPERCLASS QSystemTrayIcon
#endif

class SysTrayIcon : public STI_SUPERCLASS
{
Q_OBJECT

public:
  SysTrayIcon( bool noWindow = false, QObject *parent = 0 );
  ~SysTrayIcon();
  void setDoubleClickFunction( int );
  bool dontStart() { return _dontStart; }
  QStringList templates();
  QStringList templateTitles();
  void quickpostFromDBus( QString &, QString & );
				    
public slots:
  void configureQuickpostTemplates( QWidget *parent = 0 );
  void setCopyTitle( bool );
  void newDoc();
  void openRecentFile();
  bool handleArguments();
  void doQuit();
  void saveAll();
  void setRecentFiles( const QList<Application::recentFile> & );

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason );
  void choose( QString fname = QString() );
  void quickpost( QClipboard::Mode mode = QClipboard::Clipboard );
  void quickpostFromTemplate( int, QString, QString t = QString() );
  void setNewWindowAtStartup( bool );
  void handleResponseHeader( const QHttpResponseHeader & );
  void handleDone( bool );
  void handleHostLookupFailed();
  void abortQP();

signals:
  void quickpostTemplateTitlesUpdated( QStringList );
  void quickpostTemplatesUpdated( QStringList );

private:
  Application *qtm;
  //enum _cbtextIsURL { No, Yes, Untested };
  //enum _cbtextIsURL cbtextIsURL;
  bool _copyTitle;
  int activeTemplate;
  QAction *newWindowAtStartup;
  QAction *abortAction;
  QAction *configureTemplates;
#ifndef SUPERMENU
  QAction *quitAction;
#endif
  QList<QuickpostTemplate *> quickpostTemplateActions;
  QMenu *menu;
  QMenu *templateMenu;
  bool _newWindowAtStartup;
  _HTTP *http;
  QByteArray responseData;
  QString cbtext;
  // bool cbtextIsURL;
  int doubleClickFunction;
  bool httpBusy;
  bool templateQPActive;
  bool _dontStart;
  QStringList templateTitleList, templateList;
  QList<int> defaultPublishStatusList;
  QList<bool> copyTitleStatusList;
  QList<QStringList> assocHostLists;
  QList<Application::recentFile> recentFiles;
  QAction *recentFileActions[10];
  QAction *openRecent;
  QAction *noRecentFilesAction;
  QMenu *recentFilesMenu;
  QString userAgentString;
#ifndef DONT_USE_DBUS
  //QDBusConnection *dbus;
#endif

  void setupQuickpostTemplates();
  void doQP( QString );
  QDomElement templateElement( QDomDocument &, QString &, QString &, 
			       int &, bool &, QStringList & );
  void updateRecentFileMenu();
};

#endif
//#endif
#endif
