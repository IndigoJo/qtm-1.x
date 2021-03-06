/************************************************************************************************
 *
 * AccountsDialog.cc - Multi-account form for QTM
 *
 * Copyright (C) 2007, 2008, Matthew J Smith
 *
 * This file is part of QTM.
 * QTM program is free software; you can redistribute it and/or modify
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
 *************************************************************************************************/


#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QList>
#include <QWhatsThis>
#include <QMessageBox>
#include <QModelIndex>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTime>
#include <QHttp>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QFocusEvent>
#if QT_VERSION <= 0x040300
#include <QTextDocument>
#endif

#include <QtXml>

#include "AccountsDialog.h"
#include "ui_AccountsForm.h"
#include "return.xpm"

AccountsDialog::AccountsDialog( QList<AccountsDialog::Account> &acctList,
                                int acctIndex,
                                QWidget *parent )
: QDialog( parent )
{
  QString a;
  networkBiz = NoBusiness;
  http = new QHttp;

  setupUi( this );
  leBlogURI->installEventFilter( this );
  leLocation->installEventFilter( this );
  pbNew->setDefault( false );

  // Set up internal account lists; one for the current contents of the accounts,
  // one for backup
  accountList = acctList;
  originalAccountList = acctList;

  hboxLayout->setStretchFactor( hboxLayout->itemAt( 0 )->layout(), 2 );
  hboxLayout->setStretchFactor( hboxLayout->itemAt( 1 )->layout(), 3 );

  for( int i = 0; i < accountList.count(); i++ ) {
    a = accountList.at( i ).name;
    if( a.isEmpty() )
      lwAccountList->addItem( tr( "(No name)" ) );
    else
      lwAccountList->addItem( a );
  }
  //qDebug( "added the templates" );

  // Set up list of account widgets;
  accountWidgets << leName << cbHostedBlogType << leServer << leLocation
    << lePort << leLogin << lePassword
    << chCategoriesEnabled << chPostDateTime << chAllowComments << chAllowTB << chUseWordpressAPI;

  pbReset->setVisible( false );

  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( false );

  doingNewAccount = false;
  // setClean();
  currentRow = -1;

  addAccount = new QAction( tr( "&Add account" ), lwAccountList );
  removeAccount = new QAction( tr( "&Remove this account" ), lwAccountList );
  lwAccountList->addAction( addAccount );
  lwAccountList->addAction( removeAccount );
  lwAccountList->setContextMenuPolicy( Qt::ActionsContextMenu );

  connect( addAccount, SIGNAL( triggered( bool ) ),
           this, SLOT( doNewAccount() ) );
  connect( removeAccount, SIGNAL( triggered( bool ) ),
           this, SLOT( removeThisAccount() ) );
  connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
           this, SLOT( changeListIndex( int ) ) );

  entryDateTime = QDateTime();
  currentRow = acctIndex;
  lwAccountList->setCurrentRow( acctIndex );
}

void AccountsDialog::changeListIndex( int index )
{
  leBlogURI->clear();

  if( doingNewAccount ) {
    if( dirty )
      lwAccountList->item( (lwAccountList->count())-1 )->setText( leName->text().isEmpty() ?
                                                                  tr( "(No name)" ) :
                                                                  leName->text() );
    else {
      accountList.removeAt( currentRow );
      lwAccountList->takeItem( lwAccountList->count()-1 );
    }
  }

  doingNewAccount = false;
  currentRow = index;

  leName->setText( accountList[currentRow].name );
  leServer->setText( accountList[currentRow].server );
  leLocation->setText( accountList[currentRow].location );
  lePort->setText( accountList[currentRow].port );
  leLogin->setText( accountList[currentRow].login );
  lePassword->setText( accountList[currentRow].password );

  if( accountList[currentRow].hostedBlogType != -1 )
    cbHostedBlogType->setCurrentIndex( accountList[currentRow].hostedBlogType );	  

  chCategoriesEnabled->setCheckState( accountList[currentRow].categoriesEnabled ?
                                      Qt::Checked : Qt::Unchecked );
  chPostDateTime->setCheckState( accountList[currentRow].postDateTime ?
                                 Qt::Checked : Qt::Unchecked );
  chAllowComments->setCheckState( accountList[currentRow].allowComments ? Qt::Checked :
                             Qt::Unchecked );
  chAllowTB->setCheckState( accountList[currentRow].allowTB ? Qt::Checked : Qt::Unchecked );
  chUseWordpressAPI->setCheckState( accountList[currentRow].useWordpressAPI ?
                                    Qt::Checked : Qt::Unchecked );

  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( true );
}

void AccountsDialog::doNewAccount()
{
  QLineEdit *le;

  entryDateTime = QDateTime::currentDateTime();
  Q_FOREACH( QWidget *w, accountWidgets )
    w->setEnabled( true );

  accountList.append( Account() );

  lwAccountList->disconnect( SIGNAL( currentRowChanged( int ) ),
                             this, SLOT( changeListIndex( int ) ) );
  currentRow = -1;
  Q_FOREACH( QWidget *v, accountWidgets ) {
    le = qobject_cast<QLineEdit *>( v );
    if( le )
      le->clear();
  }

  lwAccountList->addItem( tr( "New account" ) );
  currentRow = lwAccountList->count()-1;
  lwAccountList->setCurrentRow( lwAccountList->count()-1 );
  connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
           this, SLOT( changeListIndex( int ) ) );

  doingNewAccount = true;
  leName->setFocus( Qt::OtherFocusReason );
  connect( qApp, SIGNAL( focusChanged( QWidget *, QWidget * ) ),
           this, SLOT( assignSlug() ) );

  setClean();
}

void AccountsDialog::removeThisAccount()
{
  QLineEdit *le;
  QCheckBox *ch;
  int c = lwAccountList->currentRow();

  if( lwAccountList->count() == 1 ) {
    lwAccountList->disconnect( SIGNAL( currentRowChanged( int ) ) );
    lwAccountList->clear();
    accountList.clear();

    Q_FOREACH( QWidget *w, accountWidgets ) {
      le = qobject_cast<QLineEdit *>( w );
      if( le ) {
        le->clear();
        le->setEnabled( false );
      }
      ch = qobject_cast<QCheckBox *>( w );
      if( ch ) {
        ch->setChecked( false );
        ch->setEnabled( false );
      }
    }
    currentRow = -1;
  } else {
    lwAccountList->takeItem( c );
    accountList.removeAt( c );
  }
}

void AccountsDialog::setDirty()
{
  dirty = true;
  Q_FOREACH( QWidget *w, accountWidgets )
    disconnect( w, 0, this, SLOT( setDirty() ) );
}

void AccountsDialog::setClean()
{
  dirty = false;

  Q_FOREACH( QWidget *w, accountWidgets )
    disconnect( w, 0, this, SLOT( setDirty() ) );

  Q_FOREACH( QWidget *v, accountWidgets ) {
    if( qobject_cast<QLineEdit *>( v ) )
      connect( v, SIGNAL( textChanged( const QString & ) ),
               this, SLOT( setDirty() ) );
  }
}

void AccountsDialog::acceptAccount()
{
  Account newAcct;
  if( doingNewAccount ) {
    accountList.append( currentAcct );

    if( lwAccountList->count() == 1 )
      connect( lwAccountList, SIGNAL( currentRowChanged( int ) ),
               this, SLOT( changeListIndex( int ) ) );
  } else {
    if( lwAccountList->count() ) {
      lwAccountList->item( currentRow )->setText( leName->text().isEmpty() ?
                                                  tr( "(No name)" ) : leName->text() );
    }
  }

  doingNewAccount = false;
  setClean();
}

/* Private slot method to assign a unique slug name to identify an account.
 * It's done by the second, which should ensure uniqueness as long as identical
 * accounts aren't created by a program.
 */
void AccountsDialog::assignSlug()
{
  if( accountList.count() != 0 ) {
    currentAccountId = QString( "%1_%2" ).arg( entryDateTime.toString( Qt::ISODate ) )
      .arg( leName->text().toLower()
            .replace( QRegExp( "\\s" ), "_" ) );
    accountList[currentRow].id = currentAccountId;
  }
}

void AccountsDialog::on_pbWhatsThis_clicked()
{
  QWhatsThis::enterWhatsThisMode();
}

void AccountsDialog::on_pbOK_clicked()
{
  if( doingNewAccount && !dirty )
    accountList.removeAt( currentRow );

  accept();
}

void AccountsDialog::on_leBlogURI_returnPressed()
{
  int i;
  QString uriServer, urisLocation;
  QRegExp re( "/.*\\.[shtml|dhtml|phtml|html|htm|php|cgi|pl|py]$" );

  QUrl uri = QUrl( leBlogURI->text(), QUrl::TolerantMode );
  QString uris = uri.toString();

  if( !uri.isValid() ) {
    QMessageBox::information( 0, tr( "QTM: URI not valid" ),
                              tr( "That web location is not valid." ),
                              QMessageBox::Cancel );
    return;
  }

  //bool found = false;
  //  QStringList hostedAccountStrings, hostedAccountServers, hostedAccountLocations;
  QStringList wpmuHosts;
  wpmuHosts << "wordpress.com" << "blogsome.com" << "blogs.ie"
    << "hadithuna.com" << "edublogs.com" << "weblogs.us"
    << "blogvis.com" << "blogates.com";
  /*  hostedAccountEndpoints << "@host@;xmlrpc.php"
      << "www.typepad.com;/t/api"
      << "www.squarespace.com;/do/process/external/PostInterceptor"
      << "@host@;xmlrpc.php"
      << "@host@;xmlrpc.php"
      << "@host@;xmlrpc.php"; */

  // First check for TypePad and SquareSpace
  if( uris.contains( "squarespace.com" ) ) {
    leServer->setText( "www.squarespace.com" );
    accountList[currentRow].server = "www.squarespace.com";
    leLocation->setText( "/do/process/external/PostInterceptor" );
    accountList[currentRow].location = "/do/process/external/PostInterceptor";
    return;
  }

  if( uris.contains( "typepad.com" ) ) {
    leServer->setText( "www.typepad.com" );
    accountList[currentRow].server = "www.typepad.com";
    leLocation->setText( "/t/api" );
    accountList[currentRow].location = "/t/api";
    return;
  }

  // Now check if it's a Wordpress MU host
  for( i = 0; i <= wpmuHosts.count(); i++ ) {
    if( i < wpmuHosts.count() ) {
      if( uris.contains( wpmuHosts.at( i ) ) ) {
        leServer->setText( uri.host() );
        accountList[currentRow].server = uri.host();
        leLocation->setText( "/xmlrpc.php" );
        accountList[currentRow].location = "/xmlrpc.php";
        lePort->clear();
        accountList[currentRow].port = "";
        leLogin->setFocus( Qt::TabFocusReason );
        return;
      }
    }
  }

  // Is this a self-hosted Wordpress, Textpattern or Drupal site?
  if( cbHostedBlogType->currentIndex() >= 5 &&
      cbHostedBlogType->currentIndex() <= 7 ) {
    QString endpoint;
    if( cbHostedBlogType->currentIndex() == 7 ) // i.e. Textpattern
      endpoint = "rpc/index.php";
    else
      endpoint = "xmlrpc.php";

    accountList[currentRow].server = uris.section( "//", 1, 1 ).section( "/", 0, 0 );
    leServer->setText( accountList[currentRow].server ); 

    urisLocation = uri.path();
    if( re.exactMatch( urisLocation ) )
      urisLocation = urisLocation.section( "/", 0, -2 ).append( endpoint );
    else
      urisLocation.append( urisLocation.endsWith( '/' ) ? endpoint : 
                           QString( "/%1" ).arg( endpoint ) );
    leLocation->setText( urisLocation );
    accountList[currentRow].location = urisLocation;
    return;
  }

  // Now test for an rsd.xml file 
  http->setHost( uri.host() );
  QString loc( uri.path() );
  if( re.exactMatch( loc ) )
    http->get( loc.section( "/", -2, 0, QString::SectionIncludeTrailingSep )
               .append( "rsd.xml" ) );
  else
    http->get( loc.append( loc.endsWith( '/' ) ? "rsd.xml" : "/rsd.xml" ) );

  if( qApp->overrideCursor() == 0 )
    qApp->setOverrideCursor( QCursor( Qt::BusyCursor ) );

  networkBiz = FindingRsdXml;

  connect( http, SIGNAL( requestFinished( int, bool ) ),
           this, SLOT( handleRequestFinished( int, bool ) ) );
  connect( http, SIGNAL( done( bool ) ),
           this, SLOT( handleHttpDone( bool ) ) );

}

void AccountsDialog::handleRequestFinished( int /* id */,
                                            bool error )
{
  if( !error )
    httpByteArray = http->readAll();
}

void AccountsDialog::handleHttpDone( bool error )
{
  QDomDocument rsdXml;
  QDomNodeList apis;
  QDomElement thisApi;
  QHttpResponseHeader responseHeader;
  int i;
  QUrl url;
  QString failure = tr( "QTM failed to auto-configure access to your account. "
                        "Please consult the documentation for your conent management system "
                        "or service." );

  responseHeader = http->lastResponse();

  if( !error ) {
    switch( networkBiz ) {
      case FindingRsdXml:
        if( responseHeader.statusCode() == 200 ) { /* 200 means success */
          rsdXml.setContent( httpByteArray );
          httpByteArray = QByteArray();
          if( rsdXml.documentElement().tagName() == "rsd" ) {
            apis = rsdXml.elementsByTagName( "api" );
            for( i = 0; i < apis.count(); i++ ) {
              if( apis.at( i ).toElement().attribute( "name" ) == "MetaWeblog" ) {
                url = QUrl( apis.at( i ).toElement().attribute( "apiLink" ) );
                if( url.isValid() ) {
                  leServer->setText( url.host() );
                  accountList[currentRow].server = url.host();
                  leLocation->setText( url.path() );
                  accountList[currentRow].location = url.path();
                  leLogin->setFocus();
                  break;
                }
              }
            }
          }
          else 
            QMessageBox::information( this, tr( "QTM: Failure" ), failure, QMessageBox::Cancel );
        }
        else 
          // Attempt to find tell-tale files has failed
          QMessageBox::information( this, tr( "QTM: Failure" ), failure, QMessageBox::Cancel );
        http->close();
        currentReq = QHttpRequestHeader();
        disconnect( http, SIGNAL( requestFinished( int, bool ) ), this, 0 );
        disconnect( http, SIGNAL( done( bool ) ), this, 0 );
        break;
      case FindingXmlrpcPhp:
        /*
        // If it finds xmlrpc.php, it returns a short string with a successful (200) status code
        if( responseHeader.statusCode() == 200 ) {
        leServer->setText( currentHost );
        currentHost = QString();
        leLocation->setText( QUrl( currentReq.path() ).path() );
        http->close();
        currentReq = QHttpRequestHeader();
        networkBiz = NoBusiness;
        disconnect( http, SIGNAL( requestFinished() ), this, 0 );
        disconnect( http, SIGNAL( done( bool ) ), this, 0 );
        break;
        }
        else {
        http->close();
        http->setHost( QUrl( currentReq.path() ).host() );
        http->get( QUrl( currentReq.path() ).path().replace( "xmlrpc.php", "rsd.xml" ) );
        currentReq = QHttpRequestHeader();
        networkBiz = FindingRsdXml;
        }  */
        break;
    }
  }
  else {
    QMessageBox::information( this, tr( "QTM - Network failure" ),
                              tr( "QTM could not contact the site.  Please consult the documentation "
                                  "for your content management system or service and enter the "
                                  "server and location manually." ),
                              QMessageBox::Cancel );
    http->close();
    currentReq = QHttpRequestHeader();
  }
  if( qApp->overrideCursor() != 0 )
    qApp->restoreOverrideCursor();
}

void AccountsDialog::on_leName_textEdited( const QString &newName )
{
  if( currentRow != -1 ) {
    // qDebug( "title edited" );
    lwAccountList->item( currentRow )->setText( newName.isEmpty() ?
                                                tr( "(No name)" ) : newName );
    accountList[currentRow].name = newName;
  }
}

void AccountsDialog::on_leServer_textEdited( const QString &newServ )
{
  if( currentRow != -1 )
    accountList[currentRow].server = newServ;
}

void AccountsDialog::on_leLocation_textEdited( const QString &text )
{
  if( currentRow != -1 )
    accountList[currentRow].location = text;
}

void AccountsDialog::on_lePort_textEdited( const QString &text )
{
  if( currentRow != -1 )
    accountList[currentRow].port = text;
}

void AccountsDialog::on_leLogin_textEdited( const QString &text )
{
  if( currentRow != -1 )
    accountList[currentRow].login = text;
}

void AccountsDialog::on_lePassword_textEdited( const QString &text )
{
  if( currentRow != -1 )
    accountList[currentRow].password = text;
}

void AccountsDialog::on_chCategoriesEnabled_clicked( bool )
{
  if( currentRow != -1 )
    accountList[currentRow].categoriesEnabled = chCategoriesEnabled->isChecked();
}

void AccountsDialog::on_chPostDateTime_clicked( bool )
{
  if( currentRow != -1 )
    accountList[currentRow].postDateTime = chPostDateTime->isChecked();
}

void AccountsDialog::on_chAllowComments_clicked( bool )
{
  if( currentRow != -1 )
    accountList[currentRow].allowComments = chAllowComments->isChecked();
}

void AccountsDialog::on_chAllowTB_clicked( bool )
{
  if( currentRow != -1 )
    accountList[currentRow].allowTB = chAllowTB->isChecked();
}

void AccountsDialog::on_chUseWordpressAPI_clicked( bool )
{
  if( currentRow != -1 )
    accountList[currentRow].useWordpressAPI = chUseWordpressAPI->isChecked();
}

void AccountsDialog::on_cbHostedBlogType_activated( int newIndex )
{
  switch( newIndex ) {
    case 1: // wordpress.com
      leServer->setText( "yourblog.wordpress.com" );
      leLocation->setText( "/xmlrpc.php" );
      break;
    case 2: // TypePad
      leServer->setText( "www.typepad.com" );
      leLocation->setText( "/t/api" );
      break;
    case 3: // SquareSpace
      leServer->setText( "www.squarespace.com" );
      leLocation->setText( "/do/process/external/PostInterceptor" );
      break;
    case 4: // Movable type
    case 5: // Self-hosted Wordpress
      leServer->clear();
      leLocation->clear();
      break;
    case 6: // Drupal  
    case 7: // TextPattern
      chAllowTB->setChecked( false ); // these platforms don't support it
      leServer->clear();
      leLocation->clear();
      break;
  }
  accountList[currentRow].hostedBlogType = newIndex;
}

bool AccountsDialog::eventFilter( QObject *obj, QEvent *event )
{
  // When the blog URI is typed in and Tab is pressed, the same must happen as when
  // Return is pressed, i.e. QTM must try to find the endpoint.
  if( obj == leBlogURI ) {
    switch( event->type() ) {
      case QEvent::FocusOut:
        if( !leBlogURI->text().isEmpty() ) {
          on_leBlogURI_returnPressed();
        }
      default:
        return QObject::eventFilter( obj, event );
    }
  }

  // The text in the location field must start with /; otherwise, the web server will
  // return an error.
  if( obj == leLocation ) {
    switch( event->type() ) {
      case QEvent::FocusOut:
        if( !leLocation->text().isEmpty() && !leLocation->text().startsWith( '/' ) ) {
          QString newLocation = leLocation->text().prepend( '/' );
          leLocation->setText( newLocation );
          if( currentRow != -1 )
            accountList[currentRow].location = newLocation;
        }
      default:
        return QObject::eventFilter( obj, event );
    }
  }

  return QObject::eventFilter( obj, event );
}

