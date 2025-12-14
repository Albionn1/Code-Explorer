#ifndef CODEHIGHLIGHTER_H
#define CODEHIGHLIGHTER_H
#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

struct MiniToken{
    QString text;
    QColor color;
};

class codehighlighter : public QSyntaxHighlighter{
    Q_OBJECT

public:
    explicit codehighlighter(QTextDocument* parent = nullptr, bool darkMode = true);

    void setDarkMode(bool enabled);
    QVector<MiniToken> highlightLine(const QString& line) const;

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat classFormat;
    QTextCharFormat methodFormat;
    QTextCharFormat variableFormat;
    QTextCharFormat numberFormat;
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat parameterFormat;

    void setupRules(bool darkMode);
};

#endif // CODEHIGHLIGHTER_H
