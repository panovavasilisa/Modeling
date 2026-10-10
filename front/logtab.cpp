#include "logtab.h"
#include <QVBoxLayout>

LogTab::LogTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    text = new QTextEdit(this);
    text->setReadOnly(true);
    text->setFontFamily("Consolas");
    layout->addWidget(text);
}

void LogTab::append(const QString& line) {
    text->append(line);
}

void LogTab::clear() {
    text->clear();
}
