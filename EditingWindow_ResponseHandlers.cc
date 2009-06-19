/*******************************************************************************

  QTM - Qt-based blog manager
  Copyright (C) 2006 Matthew J Smith

  This program is free software; you can redistribute it and/or modify
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

// EditingWindow_ResponseHandlers.cc - XML-RPC response handlers for QTM

#include <QtCore>
#include <QtGui>
#include <QtXml>

#include "EditingWindow.h"
#include "XmlRpcHandler.h"

void EditingWindow::blogger_getUsersBlogs( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler( currentHttpBusiness );
  QDomDocument doc;
  QDomNodeList blogNodeList;
  QDomDocumentFragment importedBlogList;

  console->insertPlainText( QString( response ) );
  cw.cbBlogSelector->disconnect( this, SLOT( changeBlog( int ) ) );
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  handler.setProtocol( currentHttpBusiness );
  reader.setContentHandler( &handler );
  reader.setErrorHandler( &handler );
  xis.setData( response );
  reader.parse( &xis );
#ifndef NO_DEBUG_OUTPUT
  // qDebug() << "just parsed blog list";
#endif

  importedBlogList = handler.returnXml();
  blogNodeList = importedBlogList.firstChildElement( "blogs" )
    .elementsByTagName( "blog" );

  cw.cbBlogSelector->clear();

  int i = blogNodeList.count();
  QString fstring = handler.faultString();
  if( !fstring.isEmpty() ) {
    statusBar()->showMessage( tr( "Could not connect - check account details & password" ), 2000 );
    addToConsole( QString( "%1\n" ).arg( fstring ) );
    cw.cbBlogSelector->setEnabled( false );
    cw.chNoCats->setEnabled( false );
    cw.cbMainCat->setEnabled( false );
    cw.cbMainCat->clear();
    cw.lwOtherCats->setEnabled( false );
    cw.lwOtherCats->clear();
  }
  else {
    if( !i ) {
      statusBar()->showMessage( tr( "No blogs found" ), 2000 );
    }
    else {
      currentAccountElement.removeChild( currentAccountElement.firstChildElement( "blogs" ) );
      //qDebug() << "appending new blogs element";
      currentAccountElement.appendChild( accountsDom.importNode( importedBlogList.firstChildElement( "blogs" ), true ) );
      //qDebug() << "done";
      if( !noAutoSave ) {
        QFile domOut( PROPERSEPS( QString( "%1/qtmaccounts2.xml" ).arg( localStorageDirectory ) ) );
        if( domOut.open( QIODevice::WriteOnly ) ) {
          QTextStream domFileStream( &domOut );
          accountsDom.save( domFileStream, 2 );
          domOut.close();
          //qDebug() << "saved";
        }
        else
          statusBar()->showMessage( tr( "Could not write to accounts file (error %1)" ).arg( (int)domOut.error() ), 2000 );
      }

      for( int a = 0; a < i; a++ ) {
        cw.cbBlogSelector->addItem( blogNodeList.at( a ).firstChildElement( "blogName" ).text(),
                                    QVariant( blogNodeList.at( a ).firstChildElement( "blogid" ).text() ) );
      }

      cw.cbBlogSelector->setCurrentIndex( 0 );
      currentBlogElement = currentAccountElement.firstChildElement( "blogs" ).firstChildElement( "blog" );
      //if( currentBlogElement.isNull() )
        //qDebug() << "cbe is null";
      currentBlogid = cw.cbBlogSelector->itemData( 0 ).toString();
      cw.cbBlogSelector->setEnabled( true );
      disconnect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
                  this, SLOT( changeBlog( int ) ) );
      connect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
               this, SLOT( changeBlog( int ) ) );
      addToConsole( accountsDom.toString( 2 ) );

      if( !initialChangeBlog ) {
        disconnect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
                    this, SLOT( changeBlog( int ) ) );
        connect( cw.cbBlogSelector, SIGNAL( activated( int ) ),
                 this, SLOT( changeBlog( int ) ) );
      }

      if( loadedEntryBlog != 999 )
        connect( this, SIGNAL( httpBusinessFinished() ),
                 this, SLOT( doInitialChangeBlog() ) );
    }
  }
#ifndef NO_DEBUG_OUTPUT
  // qDebug() << "Finished handling the output";
#endif
  if( QApplication::overrideCursor() )
    QApplication::restoreOverrideCursor();
}

void EditingWindow::metaWeblog_newPost( QByteArray response )
{
  // Returned data should only contain a single string, and no structs. Hence
  // the XmlRpcHandler is not suitable.
#ifndef NO_DEBUG_OUTPUT
  // qDebug() << "posted the piece";
#endif

  addToConsole( QString( response ) );
  if( response.contains( "<fault>" ) ) {
    statusBar()->showMessage( tr( "The submission returned a fault - see console." ), 2000 );
    if( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    QTimer::singleShot( 1000, this, SLOT( hideProgressBar() ) );
  }
  else {
    QString parsedData( response );
    entryNumber = parsedData.section( "<string>", 1, 1 )
      .section( "</string>", -2, -2 );
    if( !cw.chNoCats->isChecked() )
      connect( this, SIGNAL( httpBusinessFinished() ),
               this, SLOT( setPostCategories() ) );

    if( !entryEverSaved ) {
      if( postAsSave && cleanSave ) {
        setWindowModified( false );
        dirtyIndicator->hide();
        setDirtySignals( true );
      }
    }
  }

  addToConsole( QString( "Entry number: %1\n" ).arg( entryNumber ) );
  entryBlogged = true;
}


void EditingWindow::metaWeblog_editPost( QByteArray response )
{
  addToConsole( QString( response ) );

  if( response.contains( "<fault>" ) ) {
    statusBar()->showMessage( tr( "The submission returned a fault - see console." ), 2000 );
    QTimer::singleShot( 1000, this, SLOT( hideProgressBar() ) );
  }
  else {
    connect( this, SIGNAL( httpBusinessFinished() ),
             this, SLOT( setPostCategories() ) );
    if( !entryEverSaved ) {
      if( postAsSave && cleanSave ) {
        setWindowModified( false );
        dirtyIndicator->hide();
        setDirtySignals( true );
      }
    }
  }
  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void EditingWindow::metaWeblog_newMediaObject( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler;

#ifndef NO_DEBUG_OUTPUT
  // qDebug() << "Handling RPC response";
#endif
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  addToConsole( QString( response ) );

  if( !response.contains( "<fault>" ) ) {
    if( response.contains( "<name>url</name>" ) ) {
      remoteFileLocation = QString( response )
        .section( "<string>", 1, 1 )
        .section( "</string>", 0, 0 );
      pbCopyURL->show();
      ui.actionCopy_upload_location->setVisible( true );
      statusBar()->showMessage( tr( "Your file is here: %1" ).arg( remoteFileLocation ), 2000 );
    }
    else
      statusBar()->showMessage( tr( "The upload returned a fault." ), 2000 );
  }
  else {
    statusBar()->showMessage( tr( "The upload returned a fault." ), 2000 );
  }
#ifndef NO_DEBUG_OUTPUT
  // qDebug() << "Finished handling response";
#endif

  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void EditingWindow::mt_publishPost( QByteArray response )
{
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );
  addToConsole( QString( response ) );

  if( response.contains( "<fault>" ) )
    statusBar()->showMessage( tr( "An error occurred during rebuilding." ), 2000 );
  else
    statusBar()->showMessage( tr( "The post was published successfully." ), 2000 );

  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void EditingWindow::mt_getCategoryList( QByteArray response )
{
  QXmlInputSource xis;
  QXmlSimpleReader reader;
  XmlRpcHandler handler( currentHttpBusiness );
  QDomDocumentFragment importedCategoryList;
  QDomElement newCategoriesElement, currentCategory, currentID, currentName;
  QString xmlRpcFaultString;
  QStringList catList;

  cw.lwOtherCats->reset();

  addToConsole( response );

  handler.setProtocol( currentHttpBusiness );
  reader.setContentHandler( &handler );
  reader.setErrorHandler( &handler );
  xis.setData( response );
  reader.parse( &xis );
  while( !handler.isMethodResponseFinished() )
    QCoreApplication::processEvents();

  bool xfault = handler.fault();

  importedCategoryList = handler.returnXml();
  xmlRpcFaultString = handler.faultString();

  categoryNames = handler.returnList( "categoryName" );
  categoryIDs = handler.returnList( "categoryId" );

  QDomElement returnedCategoriesElement = importedCategoryList.firstChildElement( "categories" );
  QDomNodeList returnedCats = returnedCategoriesElement.elementsByTagName( "category" );
  int i = returnedCats.size();
  for( int j = 0; j < i; j++ )
    if( !returnedCats.at( j ).firstChildElement( "categoryId" ).isNull() &&
        !returnedCats.at( j ).firstChildElement( "categoryName" ).isNull() ) {
      catList.append( QString( "%1 ##ID:%2" )
                      .arg( returnedCats.at( j ).firstChildElement( "categoryName" ).text() )
                      .arg( returnedCats.at( j ).firstChildElement( "categoryId" ).text() ) );
    }
  if( !noAlphaCats )
    qSort( catList.begin(), catList.end(), EditingWindow::caseInsensitiveLessThan );

  if( xfault ) {
    statusBar()->showMessage( tr( "Could not connect; check account details & password" ), 2000 );
  }
  else {
    if( !i ) {
      statusBar()->showMessage( tr( "There are no categories." ) );
      cw.chNoCats->setEnabled( false );
      cw.cbMainCat->setEnabled( false );
      cw.lwOtherCats->setEnabled( false );
    }
    else {
      newCategoriesElement = accountsDom.createElement( "categories" );
      QStringList::iterator it;
      cw.cbMainCat->clear();
      cw.lwOtherCats->clear();
      for( it = catList.begin(); it != catList.end(); ++it ) {
        cw.cbMainCat->addItem( it->section( " ##ID:", 0, 0 ),
                               QVariant( it->section( " ##ID:", 1, 1 ) ) );
        cw.lwOtherCats->addItem( it->section( " ##ID:", 0, 0 ) );
        currentCategory = accountsDom.createElement( "category" );
        currentID = accountsDom.createElement( "categoryId" );
        currentID.appendChild( accountsDom.createTextNode( it->section( " ##ID:", 1 ) ) );
        currentName = accountsDom.createElement( "categoryName" );
        currentName.appendChild( accountsDom.createTextNode( it->section( " ##ID:", 0, 0 ) ) );
        currentCategory.appendChild( currentID );
        currentCategory.appendChild( currentName );
        newCategoriesElement.appendChild( currentCategory );
      }
      if( currentBlogElement.firstChildElement( "categories" ).isNull() )
        currentBlogElement.appendChild( newCategoriesElement );
      else
        currentBlogElement.replaceChild( newCategoriesElement, currentBlogElement.firstChildElement( "categories" ) );

      cw.chNoCats->setEnabled( true );
      cw.cbMainCat->setEnabled( true );
      cw.lwOtherCats->setEnabled( true );
      if( !noAutoSave ) {
        QFile domOut( PROPERSEPS( QString( "%1/qtmaccounts2.xml" ).arg( localStorageDirectory ) ) );
        if( domOut.open( QIODevice::WriteOnly ) ) {
          QTextStream domFileStream( &domOut );
          accountsDom.save( domFileStream, 2 );
          domOut.close();
          //qDebug() << "saved";
        }
        else
          statusBar()->showMessage( tr( "Could not write to accounts file (error %1)" ).arg( (int)domOut.error() ), 2000 );
      }
      /*else
        qDebug() << "no auto-save"; */
    }
  }

  addToConsole( accountsDom.toString( 2 ) );

  connect( this, SIGNAL( httpBusinessFinished() ),
           this, SIGNAL( categoryRefreshFinished() ) );

  if( QApplication::overrideCursor() != 0 )
    QApplication::restoreOverrideCursor();
}

void EditingWindow::wp_newCategory( QByteArray response )
{
  disconnect( this, SIGNAL( httpBusinessFinished() ), 0, 0 );

  // Parse the XML

  addToConsole( QString( response ) );

  QApplication::restoreOverrideCursor();
}

