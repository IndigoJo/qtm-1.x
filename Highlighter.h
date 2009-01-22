// Highlighter.h - Syntax highlighter for HTML tags in QTM documents
//
// Written by Matthew J Smith
//
// Copyright 2008 Matthew J Smith
// Distributed under GNU General Public License, version 2 or later

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QString>
#include <QTextCharFormat>

class Highlighter : public QSyntaxHighlighter
{
Q_OBJECT

public:
  Highlighter::Highligher( QTextDocument *parent = 0 );

protected:
  virtual void highlightBlock( const QString & );

private:
  struct HighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QRegExp commentStartExpression;
  QRegExp commentEndExpression;

  QTextCharFormat htmlTagFormat;
  QTextCharFormat htmlEntityFormat;
  QTextCharFormat htmlCommentFormat;
};


#endif
