/***************************************************************************
 * PrefsDialog.cc - Preferences dialog for QTM
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


#include "PrefsDialog.h"

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QColorDialog>
#include <QString>
#include <QHostInfo>
#include <QMessageBox>
#include <QWhatsThis>
#include <QLabel>

PrefsDialog::PrefsDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  //nvh = false;

  //cbBlogType->setCurrentIndex( cbBlogType->count()-1 );

#ifdef Q_WS_MAC
#if QT_VERSION >= 0x040200
  // The STI double/single click functionality does not work on a Mac,
  // so these widgets are made invisible
  label_10->setVisible( false );
  cbSTI2ClickFunction->setVisible( false );
#endif
#endif
}

void PrefsDialog::on_pbBrowse_clicked()
{
  QString dir = 
    QFileDialog::getExistingDirectory( 0,
				       tr( "Choose a directory" ),
				       "/home" );

  if( !dir.isEmpty() )
    leLocalDir->setText( dir );		             
}

void PrefsDialog::on_pbWhatsThis_clicked() // slot
{
  QWhatsThis::enterWhatsThisMode();
}

void PrefsDialog::on_tbEditorBgColor_clicked() // slot
{
  QPalette palette = lEditorExampleText->palette();
  QColor color = QColorDialog::getColor( palette.color( QPalette::Window ), this );
  palette.setColor( QPalette::Window, color );
  lEditorExampleText->setPalette( palette );
}

void PrefsDialog::on_tbEditorFgColor_clicked() // slot
{
  QPalette palette = lEditorExampleText->palette();
  QColor color = QColorDialog::getColor( palette.color( QPalette::WindowText ), this );
  palette.setColor( QPalette::WindowText, color );
  lEditorExampleText->setPalette( palette );
}

void PrefsDialog::on_tbPreviewBgColor_clicked() // slot
{
  QPalette palette = lPreviewExampleText->palette();
  QColor color = QColorDialog::getColor( palette.color( QPalette::Window ), this );
  palette.setColor( QPalette::Window, color );
  lPreviewExampleText->setPalette( palette );
}

void PrefsDialog::on_tbPreviewFgColor_clicked() // slot
{
  QPalette palette = lPreviewExampleText->palette();
  QColor color = QColorDialog::getColor( palette.color( QPalette::WindowText ), this );
  palette.setColor( QPalette::WindowText, color );
  lPreviewExampleText->setPalette( palette );
}

void PrefsDialog::on_tbConsoleBgColor_clicked() // slot
{
  QPalette palette = lConsoleExampleText->palette();
  QColor color = QColorDialog::getColor( palette.color( QPalette::Window ), this );
  palette.setColor( QPalette::Window, color );
  lConsoleExampleText->setPalette( palette );
}

void PrefsDialog::on_tbConsoleFgColor_clicked() // slot
{
  QPalette palette = lConsoleExampleText->palette();
  QColor color = QColorDialog::getColor( palette.color( QPalette::WindowText ), this );
  palette.setColor( QPalette::WindowText, color );
  lConsoleExampleText->setPalette( palette );
}

/*
void PrefsDialog::on_pbForget_clicked()
{
  leServer->clear();
  leLocation->clear();
  leLogin->clear();
  lePort->clear();
  lePassword->clear();
  cbBlogType->setCurrentIndex( 4 );
}

void PrefsDialog::on_okButton_clicked() // slot
{
  QHostInfo::lookupHost( leServer->text(), this, SLOT( handleHostInfo( QHostInfo ) ) );
 }

void PrefsDialog::handleHostInfo( const QHostInfo &hostInfo )
{
  if( hostInfo.error() == QHostInfo::NoError ) {
    nvh = false;
    accept();
  }
  else {
    if( QMessageBox::question( this, tr( "Cannot find host" ),
			   tr( "Cannot find the host you specified:\n%1\n\n"
			       "Go back and correct it?" )
			   .arg( leServer->text() ),
			   tr( "&Yes" ), tr( "&No" ),
			       QString(), 0, 1 ) ) {
      nvh = true;
      accept();
    }
  }
} 
*/

