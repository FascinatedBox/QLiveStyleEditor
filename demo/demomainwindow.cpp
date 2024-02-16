#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

#include "demomainwindow.h"

DemoMainWindow::DemoMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *w = new QWidget;
    QFormLayout *formLayout = new QFormLayout;
    QLineEdit *nameLineEdit = new QLineEdit;
    QLineEdit *faveColorEdit = new QLineEdit;
    QLineEdit *phoneNumberEdit = new QLineEdit;
    QSpinBox *ageSpinBox = new QSpinBox;

    formLayout->addRow(tr("Name:"), nameLineEdit);
    formLayout->addRow(tr("Favorite Color:"), faveColorEdit);
    formLayout->addRow(tr("Phone Number:"), phoneNumberEdit);
    formLayout->addRow(tr("Age:"), ageSpinBox);

    w->setLayout(formLayout);

    setCentralWidget(w);
    setWindowTitle("QLiveStyleEditor Demo");
}
