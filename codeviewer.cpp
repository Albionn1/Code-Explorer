#include "codeviewer.h"
#include "codeeditor.h"
#include "codehighlighter.h"

#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QFileDialog>
#include <qscrollbar.h>
#include <qtoolbutton.h>
#include <QLabel>

CodeViewer::CodeViewer(QWidget* parent)
    : QWidget(parent),
    editor_(new CodeEditor(this)),
    highlighter_(new codehighlighter(editor_->document(), false))
{
    editor_->setReadOnly(true);
    editor_->setFont(QFont("Consolas", 11));
    editor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor_->setAutoFillBackground(true);

    editor_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    editor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // MAIN LAYOUT
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    // --- FIND BAR ---
    findbar_ = new QWidget(this);
    findbar_->setVisible(false);

    QHBoxLayout* findLayout = new QHBoxLayout(findbar_);
    findLayout->setContentsMargins(4, 4, 4, 4);

    findField_ = new QLineEdit(findbar_);
    findField_->setPlaceholderText("Find...");

    QLabel* matchCountLabel = new QLabel("0 of 0", findbar_);
    matchCountLabel_ = matchCountLabel;

    findLayout->addWidget(matchCountLabel_);
    findLayout->addWidget(findField_);

    connect(findField_, &QLineEdit::textChanged, this, [this]() {
        updateHighlights();
    });

    QToolButton* nextBtn = new QToolButton(findbar_);
    nextBtn->setText("Next");
    findLayout->addWidget(nextBtn);

    QToolButton* prevBtn = new QToolButton(findbar_);
    prevBtn->setText("Prev");
    findLayout->addWidget(prevBtn);

    QToolButton* regexBtn = new QToolButton(findbar_);
    regexBtn->setText(".*");
    regexBtn->setCheckable(true);
    regexBtn->setToolTip("Enable regex search");
    findLayout->addWidget(regexBtn);

    QToolButton* caseBtn = new QToolButton(findbar_);
    caseBtn->setText("Aa");
    caseBtn->setCheckable(true);
    caseBtn->setToolTip("Case sensitive");
    findLayout->addWidget(caseBtn);

    QToolButton* closeBtn = new QToolButton(findbar_);
    closeBtn->setText("X");
    findLayout->addWidget(closeBtn);

    // --- REPLACE BAR TOGGLE ---
    QToolButton* showReplaceBtn = new QToolButton(findbar_);
    showReplaceBtn->setText("Replace ▼");
    showReplaceBtn->setCheckable(true);
    showReplaceBtn->setToolTip("Show replace options");
    findLayout->addWidget(showReplaceBtn);

    // --- REPLACE BAR ---
    replaceBar_ = new QWidget(this);
    replaceBar_->setVisible(false);

    QHBoxLayout* replaceLayout = new QHBoxLayout(replaceBar_);
    replaceLayout->setContentsMargins(4, 4, 4, 4);

    replaceField_ = new QLineEdit(replaceBar_);
    replaceField_->setPlaceholderText("Replace...");
    replaceLayout->addWidget(replaceField_);

    QToolButton* replaceBtn = new QToolButton(replaceBar_);
    replaceBtn->setText("Replace");
    replaceLayout->addWidget(replaceBtn);

    QToolButton* replaceAllBtn = new QToolButton(replaceBar_);
    replaceAllBtn->setText("Replace All");
    replaceLayout->addWidget(replaceAllBtn);

    // --- BUTTON CONNECTIONS ---
    connect(nextBtn, &QToolButton::clicked, this, &CodeViewer::findNext);
    connect(prevBtn, &QToolButton::clicked, this, &CodeViewer::findPrevious);
    connect(closeBtn, &QToolButton::clicked, this, &CodeViewer::hideFindBar);

    connect(regexBtn, &QToolButton::toggled, this, [this](bool checked) {
        regexEnabled_ = checked;
        updateHighlights();
    });

    connect(caseBtn, &QToolButton::toggled, this, [this](bool checked) {
        caseSensitive_ = checked;
        updateHighlights();
    });

    connect(replaceBtn, &QToolButton::clicked, this, &CodeViewer::replaceOne);
    connect(replaceAllBtn, &QToolButton::clicked, this, &CodeViewer::replaceAll);

    connect(showReplaceBtn, &QToolButton::toggled, this,
            [this, showReplaceBtn](bool checked)
            {
                replaceBar_->setVisible(checked);
                showReplaceBtn->setText(checked ? "Replace ▲" : "Replace ▼");
            });

    // Insert Find + Replace bars
    layout->insertWidget(0, findbar_);
    layout->insertWidget(1, replaceBar_);

    // --- MINIMAP + EDITOR ---
    minimap_ = new MiniMap(this);
    minimap_->syncToEditor(editor_);
    minimap_->setFixedWidth(120); // CHANGE THE WIDTH OF THE MINIMAP HERE
    minimap_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(editor_);
    hLayout->addWidget(minimap_);

    layout->addLayout(hLayout);

    // INITIAL VISIBLE REGION
    minimap_->updateVisibleRegion(
        editor_->verticalScrollBar()->value(),
        editor_->verticalScrollBar()->pageStep()
        );

    // SCROLL SYNC
    connect(editor_->verticalScrollBar(), &QScrollBar::rangeChanged,
            this, [this]() {
                minimap_->updateVisibleRegion(
                    editor_->verticalScrollBar()->value(),
                    editor_->verticalScrollBar()->pageStep()
                    );
            });

    connect(editor_->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int value) {
                minimap_->updateVisibleRegion(
                    value,
                    editor_->verticalScrollBar()->pageStep()
                    );
            });

    setDarkMode(false);
}

void CodeViewer::loadFile(const QString& path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        editor_->setPlainText(in.readAll());
    }
}

void CodeViewer::setDarkMode(bool enabled)
{
    if (highlighter_) {
        highlighter_->setDarkMode(enabled);
    }

    QPalette p = editor_->palette();

    if (enabled) {
        p.setColor(QPalette::Base, QColor(42,42,42));        // editor background
        p.setColor(QPalette::Text, Qt::white);               // text color
        p.setColor(QPalette::Highlight, QColor(140,140,140).lighter());
        p.setColor(QPalette::HighlightedText, Qt::black);
    } else {
        p.setColor(QPalette::Base, QColor(255,255,255));     // editor background
        p.setColor(QPalette::Text, QColor(30,30,30));        // text color
        p.setColor(QPalette::Highlight, QColor(100,150,255));
        p.setColor(QPalette::HighlightedText, Qt::white);
    }

    editor_->setPalette(p);
    editor_->setAutoFillBackground(true);

    editor_->updateLineNumberAreaWidth(0);
    editor_->viewport()->update();
}

void CodeViewer::setReadOnly(bool enabled)
{
    editor_->setReadOnly(enabled);

    if (enabled) {
        editor_->setCursorWidth(0);   // hide caret
    } else {
        editor_->setCursorWidth(2);   // normal caret
    }
}

bool CodeViewer::save()
{
    if (filePath_.isEmpty())
        return false; // should call Save As instead

    QFile file(filePath_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << editor_->toPlainText();
    return true;
}

bool CodeViewer::saveAs(QWidget* parent)
{
    QString newPath = QFileDialog::getSaveFileName(parent, "Save As", filePath_);
    if (newPath.isEmpty())
        return false;

    filePath_ = newPath;
    return save();
}

void CodeViewer::showFindBar()
{
    findbar_->setVisible(true);
    findField_->setFocus();
    findField_->selectAll();
}

void CodeViewer::hideFindBar()
{
    findbar_->setVisible(false);
    editor_->setFocus();
}

void CodeViewer::findNext()
{
    QString text = findField_->text();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags;

    if (caseSensitive_)
        flags |= QTextDocument::FindCaseSensitively;

    if (regexEnabled_) {
        QRegularExpression re(text);
        editor_->find(re, flags);
    } else {
        editor_->find(text, flags);
    }
    updateHighlights();
}

void CodeViewer::findPrevious()
{
    QString text = findField_->text();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags = QTextDocument::FindBackward;

    if (caseSensitive_)
        flags |= QTextDocument::FindCaseSensitively;

    if (regexEnabled_) {
        QRegularExpression re(text);
        editor_->find(re, flags);
    } else {
        editor_->find(text, flags);
    }
    updateHighlights();
}

void CodeViewer::updateHighlights()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QString pattern = findField_->text();
    if (pattern.isEmpty()) {
        editor_->setExtraSelections(extraSelections);
        matchCountLabel_->setText("0 of 0");
        return;
    }

    QTextDocument* doc = editor_->document();
    QTextCursor cursor(doc);

    QTextDocument::FindFlags flags;
    if (caseSensitive_)
        flags |= QTextDocument::FindCaseSensitively;

    // Regex handling
    QRegularExpression re;
    if (regexEnabled_) {
        re = QRegularExpression(pattern);
        if (!re.isValid()) {
            matchCountLabel_->setText("0 of 0");
            editor_->setExtraSelections(extraSelections);
            return;
        }
    }

    int totalMatches = 0;
    int currentMatchIndex = -1;

    // Track current cursor position
    int cursorPos = editor_->textCursor().selectionStart();

    while (!cursor.isNull() && !cursor.atEnd()) {

        if (regexEnabled_) {
            cursor = doc->find(re, cursor, flags);
        } else {
            cursor = doc->find(pattern, cursor, flags);
        }

        if (!cursor.isNull()) {
            totalMatches++;

            QTextCursor current = editor_->textCursor();
            if (cursor.selectionStart() <= current.selectionStart() &&
                cursor.selectionEnd()   >= current.selectionEnd())
            {
                currentMatchIndex = totalMatches;
            }

            QTextEdit::ExtraSelection sel;
            sel.cursor = cursor;
            sel.format.setBackground(QColor(255, 230, 128));
            sel.format.setForeground(Qt::black);

            extraSelections.append(sel);
        }
    }


    if (totalMatches == 0) {
        matchCountLabel_->setText("0 of 0");
    }
    else if (currentMatchIndex == -1) {
        matchCountLabel_->setText(QString("0 of %1").arg(totalMatches));
    }
    else {
        matchCountLabel_->setText(
            QString("%1 of %2").arg(currentMatchIndex).arg(totalMatches)
            );
    }

    editor_->setExtraSelections(extraSelections);
}

void CodeViewer::replaceOne()
{
    QString findText = findField_->text();
    QString replaceText = replaceField_->text();
    if (findText.isEmpty())
        return;

    QTextCursor cursor = editor_->textCursor();

    // If nothing is selected, find next match first
    if (!cursor.hasSelection()) {
        findNext();
        cursor = editor_->textCursor();
        if (!cursor.hasSelection())
            return;
    }

    cursor.insertText(replaceText);

    findNext();

    updateHighlights();
}

void CodeViewer::replaceAll()
{
    QString findText = findField_->text();
    QString replaceText = replaceField_->text();
    if (findText.isEmpty())
        return;

    QTextDocument* doc = editor_->document();
    QTextCursor cursor(doc);

    QTextDocument::FindFlags flags;
    if (caseSensitive_)
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;

    editor_->blockSignals(true); // prevent flicker

    if (regexEnabled_) {
        QRegularExpression re(findText);
        if (!re.isValid())
            return;

        while (!(cursor = doc->find(re, cursor, flags)).isNull()) {
            cursor.insertText(replaceText);
            count++;
        }
    } else {
        while (!(cursor = doc->find(findText, cursor, flags)).isNull()) {
            cursor.insertText(replaceText);
            count++;
        }
    }

    editor_->blockSignals(false);

    updateHighlights();
}
