#ifndef GENERATEADDRESSESDIALOG_H
#define GENERATEADDRESSESDIALOG_H

#include <QDialog>

namespace Ui {
class GenerateAddressesDialog;
}

class GenerateAddressesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateAddressesDialog(uint32_t numberOfAddressesGenerated, QWidget *parent);
    ~GenerateAddressesDialog();

signals:
    void generateRequested(int count);

private:
    Ui::GenerateAddressesDialog *ui;
};

#endif // GENERATEADDRESSESDIALOG_H
