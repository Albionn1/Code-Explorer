#include "codehighlighter.h"

codehighlighter::codehighlighter(QTextDocument* parent, bool darkMode)
    : QSyntaxHighlighter(parent) {
    // setupRules(darkMode);
    setDarkMode(darkMode);
}

void codehighlighter::setDarkMode(bool enabled) {
    highlightingRules.clear();
    setupRules(enabled);
    rehighlight(); // force refresh
}

void codehighlighter::setupRules(bool darkMode) {
    HighlightingRule rule;

    // Palette
    QColor keywordColor   = darkMode ? QColor("#4FC3F7") : QColor("#1565C0");
    QColor commentColor   = darkMode ? QColor("#81C784") : QColor("#2E7D32");
    QColor stringColor    = darkMode ? QColor("#F48FB1") : QColor("#AD1457");
    QColor variableColor  = darkMode ? QColor("#FFD54F") : QColor("#FF8F00");
    QColor methodColor    = darkMode ? QColor("#64B5F6") : QColor("#0D47A1");
    QColor classColor     = darkMode ? QColor("#BA68C8") : QColor("#6A1B9A");
    QColor numberColor    = darkMode ? QColor("#FF7043") : QColor("#E65100");
    QColor preprocColor   = darkMode ? QColor("#FFEB3B") : QColor("#F57F17");
    QColor paramColor     = darkMode ? QColor("#FFB74D") : QColor("#EF6C00");

    // Keywords
    keywordFormat.setForeground(keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywords = {"class","const","int","float","return","if","else"};
    for (const QString& word : keywords) {
        rule.pattern = QRegularExpression("\\b" + word + "\\b");
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Single-line comments
    commentFormat.setForeground(commentColor);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Strings
    stringFormat.setForeground(stringColor);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Variables
    variableFormat.setForeground(variableColor);
    variableFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("\\b(?:int|float|double|QString)\\s+(\\w+)\\b");
    rule.format = variableFormat;
    highlightingRules.append(rule);

    // Methods
    methodFormat.setForeground(methodColor);
    methodFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b\\w+(?=\\()");
    rule.format = methodFormat;
    highlightingRules.append(rule);

    // Classes
    classFormat.setForeground(classColor);
    classFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b[A-Z][A-Za-z0-9_]*\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Numbers
    numberFormat.setForeground(numberColor);
    numberFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b\\d+\\b"); // integers
    rule.format = numberFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression("\\b\\d+\\.\\d+\\b"); // floats
    rule.format = numberFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression("0x[0-9A-Fa-f]+"); // hex
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Preprocessor
    preprocessorFormat.setForeground(preprocColor);
    preprocessorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\s*#\\w+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // Parameters
    parameterFormat.setForeground(paramColor);
    parameterFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("\\(([^)]*)\\)");
    rule.format = parameterFormat;
    highlightingRules.append(rule);

    // Multi-line comments setup
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression   = QRegularExpression("\\*/");
}

void codehighlighter::highlightBlock(const QString& text) {
    for (const HighlightingRule& rule : qAsConst(highlightingRules)) {
        auto matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            auto match = matchIterator.next();

            if (rule.pattern.pattern() == "\\(([^)]*)\\)") {
                int start = match.capturedStart(1);
                int length = match.capturedLength(1);
                setFormat(start, length, rule.format);
            } else {
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

    // Multi-line comments
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        auto endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int commentLength;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

// QVector<MiniToken> codehighlighter::highlightLine(const QString& line) const
// {
//     QVector<MiniToken> tokens;

//     int i = 0;
//     while (i < line.length())
//     {
//         // Example: keyword
//         auto m = keywordRegex.match(line, i);
//         if (m.hasMatch() && m.capturedStart() == i) {
//             tokens.append({ m.captured(), keywordColor });
//             i += m.capturedLength();
//             continue;
//         }

//         // Example: string
//         auto s = stringRegex.match(line, i);
//         if (s.hasMatch() && s.capturedStart() == i) {
//             tokens.append({ s.captured(), stringColor });
//             i += s.capturedLength();
//             continue;
//         }

//         // Default
//         tokens.append({ QString(line[i]), defaultColor });
//         i++;
//     }

//     return tokens;
// }
