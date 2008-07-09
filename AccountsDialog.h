/***************************************************************************
 * AccountsDialog.h - Multi-account form for QTM
 *
 * Copyright (C) 2008, Matthew J Smith
 *
 * This file is part of QTM.
 * QTM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (version 2), as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/

#include <QtGlobal>
#if QT_VERSION >= 0x040200

#include "ui_AccountsForm.h"

#include <QStringList>
#include <QList>
#include <QDialog>

class QDialog;
class QWidget;
class QAction;

class AccountsDialog : public QDialog, public Ui::AccountsForm
{
  Q_OBJECT

public:
  AccountsDialog( QList<Account> &, QWidget *parent = 0 );
  QList<Account> accounts() { return accountList; }
  /*QStringList templateTitles() { return _templateTitles; }
  QStringList templateStrings() { return _templateStrings; }
  QList<int> defaultPublishStates() { return _defaultPublishStates; }
  QList<bool> copyTitleStates() { return _copyTitleStates; }
  QList<QStringList> compileAssocHostLists( QStringList & );
  QStringList compileAssocHostStrings( QList<QStringList> & );
  QList<QStringList> assocHostLists(); */
  typedef struct _acct {
    QString id;
    QString name;
    QString server;
    QString location;
    QString port;
    QString login;
    QString password;
    bool categoriesEnabled;
    bool postDateTime;
    bool comments;
    bool trackback;
  } Account;

private:
  void acceptAccount();
  QList<Account> accountList, originalAccountList;
  Account currentAcct;
  QList<QWidget *> accountWidgets;
  bool doingNewAccount;
  bool dirty;
  int currentRow;
  QString currentAccountId;
  QAction *addAccount;
  QAction *removeAccount;
  Account currentAccount;
  QDateTime entryDateTime;

private slots:
  void changeListIndex( int );
  void doNewAccount();
  void setDirty();
  void setClean();
  void assignSlug();
  void on_pbNew_clicked() { doNewAccount(); }
  void removeThisAccount();
  // void on_pbAccept_clicked() { acceptTemplate(); }
  void on_pbWhatsThis_clicked();
  void on_pbOK_clicked();
  void on_pbCancel_clicked() { reject(); }
  void on_leName_textEdited( const QString & );
  void on_leServer_textEdited( const QString & );
  void on_leLocation_textEdited( const QString & );
  void on_lePort_textEdited( const QString & );
  void on_leLogin_textEdited( const QString & );
  void on_lePassword_textEdited( const QString & );
  void on_cbHostedBlogType_activated( int );
  void on_chCategories_toggled( bool );
  void on_chPostDateTime_toggled( bool );
  void on_chComments_toggled( bool );
  void on_chTB_toggled( bool );
};

#endif